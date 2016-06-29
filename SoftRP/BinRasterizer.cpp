#include "BinRasterizer.h"
#include "SIMDInclude.h"
#include "MathCommon.h"
#include "FVector.h"
#include "FMatrix.h"

using namespace SoftRP;
using namespace Math;

void BinRasterizer::setRenderTarget(RenderTarget* renderTarget) {
	bool changed = renderTarget != this->renderTarget() ||
		renderTarget->width() != m_renderTargetWidth ||
		renderTarget->height() != m_renderTargetHeight;
	Rasterizer::setRenderTarget(renderTarget);
	if (!changed)
		return;
	m_renderTargetWidth = this->renderTarget()->width();
	m_renderTargetHeight = this->renderTarget()->height();
	m_tilesPerWidth = static_cast<unsigned int>(std::ceil(static_cast<float>(m_renderTargetWidth) / TILE_WIDTH));
	m_tilesPerHeight = static_cast<unsigned int>(std::ceil(static_cast<float>(m_renderTargetHeight) / TILE_HEIGHT));
	m_binsCount = m_tilesPerWidth*m_tilesPerHeight;
	m_bins.resize(m_binsCount);
	size_t tileId = 0;
	for (unsigned int y = 0; y < m_renderTargetHeight; y += TILE_HEIGHT) {
		for (unsigned int x = 0; x < m_renderTargetWidth; x += TILE_WIDTH, tileId++) {
			Bin& bin = m_bins[tileId];
			bin.xMin = static_cast<int32_t>(x);
			bin.xMax = static_cast<int32_t>(std::min(x + TILE_WIDTH, m_renderTargetWidth));
			bin.yMin = static_cast<int32_t>(y);
			bin.yMax = static_cast<int32_t>(std::min(y + TILE_HEIGHT, m_renderTargetHeight));
		}
	}
}

//helper functions forward declarations
static bool intersectAABB(int32_t xMin1, int32_t xMax1, int32_t yMin1, int32_t yMax1,
						  int32_t xMin2, int32_t xMax2, int32_t yMin2, int32_t yMax2);

template<int32_t fractionalSize>
static int32_t fixedFromFloat(float x);
template<>
static int32_t fixedFromFloat<4>(float x);

template<int32_t fractionalSize>
static float fixedToFloat(int32_t fixed);
template<>
static float fixedToFloat<4>(int32_t fixed);


/*
After the transformation to Screen space, each triangle's AABB (axis-aligned bounding box) is computed and binning is performed. 
Binning consists of finding the set of bins that intersect the AABB of a triangle and add the latter to their list, so it can 
be rasterized.
Triangles are rasterized inside a bin with the aid of edge functions, which are used to determine if a given point (pixel)
is inside the triangle.

//basics
Edge function : E(v0, v1, p)
Classify a point p with respect to the edge v0v1.
Returns the determinant of the matrix  | v0.x	v1.x	p.x |
									   | v0.y   v1.y	p.y |
									   |  1		 1		 1  |
									= v0.x(v1.y - p.y) - v1.x(v0.y - p.y) + p.x(v0.y - v1.y)
									= v0.x(v1.y) - v0.x(p.y) - v1.x(v0.y) + v1.x(p.y) + p.x(v0.y) - p.x(v1.y)
									= (v0.y - v1.y)p.x + (v1.x - v0.x)p.y + v0.x(v1.y) - v1.x(v0.y)									

which is positive if p is on the left side of the edge, negative on the right, and equal to zero if it lies on the edge.
What stated above is true in a coordinate system where the y-axis goes up; here the sign needs to be flipped.

For every triangle t = (v0, v1, v2), with the vertices ordered counter-clockwise, and point p:
area(t) = E(v0, v1, v2)/2;
alpha = E(v1, v2, p), beta = E(v2, v0, p), gamma = E(v0, v1, p)

p = (alpha*v0 + beta*v1 + gamma*v2)/(alpha + beta + gamma)
  = (alpha*v0 + beta*v1 + gamma*v2)/E(v0, v1, v2)
  = (alpha*v0 + beta*v1 + gamma*v2)/(2*area(t))

p is inside t (or lies on one of its edge) iff alpha, beta and gamma >= 0

Also, A = alpha/(2*area(t)), B = beta/(2*area(t)) and C = gamma/(2*area(t)) are the barycentric coordinates of p.

The rasterization consists of iterating over all the pixels inside a bin, and for a given pixel with position p evaluating
alpha, beta and gamma to determine if it is inside the triangle.
This can be done very efficently computing the edge functions incrementally, which is possible because moving from one pixel 
to an adjacent one, the increment in position is constant ( 1 in the x-axis or 1 in the y-axis).
Ex. on the x-axis
p = (x, y), p1 = (x + 1, y) -> alpha_p = (v1.y - v2.y)x + (v2.x - v1.x)y + v1.x(v2.y) - v2.x(v1.y)
							   alpha_p1 = (v1.y - v2.y)(x + 1) + (v2.x - v1.x)y + v1.x(v2.y) - v2.x(v1.y)
							   alpha_p1 - alpha_p = v1.y - v2.y, 
							   which is constant across the entire triangle and can be computed once

As before noted, here the alpha, beta and gamma signs need to be flipped because of the y-axis. The increments are actually 
decrements: 
alpha_p = -(v1.y - v2.y)x - (v2.x - v1.x)y - v1.x(v2.y) + v2.x(v1.y)
alpha_p1 = -(v1.y - v2.y)(x + 1) - (v2.x - v1.x)y - v1.x(v2.y) + v2.x(v1.y))
alpha_p1 - alpha_p = -(v1.y - v2.y)

//coordinates precision and sub-pixel accuracy

Coordinates and computations are done with 32-bit integers using fixed-point arithmetic with 4 bits for the fractional part.
This allows to achieve sub-pixel accuracy and take advantage of the more efficient (with respect to floating-point's) 
integer's arithmetic.

//fill convention

In order to avoid overdraw on the edges, the Top-Left fill convention is used.
Assuming the y-axis points downward and the x-axis rightward,
if the triangle is in counter-clockwise order, an edge e = (x0, y0)(x1, y1) is:
-A top edge if (y1-y0) == 0 and (x1 - x0) < 0
-A left edge if (y1-y0) > 0
The pixels on an not-Top-Left edge are not drawn, which means the "inside test", for every pixel, consists of testing if
alpha, beta or gamma are >= 0 or > 0 depending on the nature of the respective edge.
For instance: alpha > 0 && beta >= 0 && gamma >= 0 if the "alpha edge" is not Top-Left and the others are.
Because alpha, beta and gamma are integers, the test (x > 0) is equivalent to (x >= 1) and so to (x-1>=0). 
Substracting 1 to alpha, beta or gamma, if they refer to a not-Top-Left edge, allows to test 
with >= 0 independently of the nature of the edge. To do this, a bias is calculated for alpha, beta and gamma that 
is added at the time of test.


//References
https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
http://forum.devmaster.net/t/advanced-rasterization/6145
http://cse.taylor.edu/~zbethel/MSR/ModernApproachToSR.pdf
*/
#ifdef SOFTRP_MULTI_THREAD
ThreadPool::Fence BinRasterizer::rasterizeTriangles(const std::vector<Vertex>& vertices, const std::vector<uint64_t>& indices,
													size_t instance, ThreadPool& threadPool)
#else
void BinRasterizer::rasterizeTriangles(const std::vector<Vertex>& vertices, const std::vector<uint64_t>& indices,
									   size_t instance)
#endif
														{
	
	const size_t triangleCount = indices.size() / 3;
	if (triangleCount == 0)
#ifdef SOFTRP_MULTI_THREAD
		return threadPool.addFence();
#else
		return;
#endif

	m_triangles.resize(triangleCount);
	const auto vertexCount = vertices.size();
	m_transformedVertices.resize(vertexCount);

	/*
	transform vertices to Screen space, keep 1/w for implementing perspective correct interpolation.	
	*/
	const FMatrix viewPortTransform = createFM(viewPort()->getTransform());
	for (size_t i = 0; i < vertexCount; i++) {
		TransformedVertex& transformedVertex = m_transformedVertices[i];		
		const Vector4& srcPos = vertices[i].position();
		const float invW = 1.0f / srcPos[3];
		transformedVertex.invW = invW;
		/*
		apply the viewport transformation. because it is linear,
		the persective divide can be done after its application.		
		*/
		FVector dest = mulFM(viewPortTransform, createFV(srcPos));		
		const FVector mulVec = createFV(invW, invW, invW, invW);
		//perspective divide
		dest = mulFV(dest, mulVec);
		transformedVertex.position = createVector4FV(dest);
	}
	
	//binning			
	const std::vector<Vertex>* verticesPtr = &vertices;
		
	for (size_t i = 0, index = 0; i < triangleCount; i++, index += 3) {

		Triangle& t = m_triangles[i];
		t.i0 = indices[index];
		t.i1 = indices[index + 1];
		t.i2 = indices[index + 2];

		const TransformedVertex& v0 = m_transformedVertices[t.i0];
		const TransformedVertex& v1 = m_transformedVertices[t.i1];
		const TransformedVertex& v2 = m_transformedVertices[t.i2];

		//convert coordinates to fixed-point notation
		const int32_t v0x = fixedFromFloat<4>(v0.position[0]);
		const int32_t v0y = fixedFromFloat<4>(v0.position[1]);
		const int32_t v1x = fixedFromFloat<4>(v1.position[0]);
		const int32_t v1y = fixedFromFloat<4>(v1.position[1]);
		const int32_t v2x = fixedFromFloat<4>(v2.position[0]);
		const int32_t v2y = fixedFromFloat<4>(v2.position[1]);
		
		/*
		compute gamma decrements here because are part of the area computation.
		the area is required to obtain barycentric coordinates and also determine 
		is the vertices are ordered counter-clockwise, i.e. determine is the triangle
		is front face
		*/
		const int32_t gammaXdecr = -(v0y - v1y);
		const int32_t gammaYdecr = -(v1x - v0x);

		const int32_t temp = -v0x*v1y + v1x*v0y;
		const int32_t twiceArea = 2 * (gammaXdecr*v2x + gammaYdecr*v2y + temp);
		if (twiceArea <= 0)
			//discard triangle because back face or too small
			continue;
		
		t.twiceArea = fixedToFloat<4>(twiceArea);

		//compute alpha and beta decrements
		t.alphaXdecr = -(+v1y - v2y);
		t.alphaYdecr = -(v2x - v1x);
		t.betaXdecr = -(v2y - v0y);
		t.betaYdecr = -(v0x - v2x);
		t.gammaXdecr = gammaXdecr;
		t.gammaYdecr = gammaYdecr;

		//compute fixed parts of the edge functions
		t.alpha0 = -v1x * v2y + v2x * v1y;
		t.beta0 = -v2x*v0y + v0x*v2y;
		t.gamma0 = temp;
		
		//compute biases for Top-Left fill convention		
		t.alphaBias = (t.alphaXdecr > 0 || (t.alphaXdecr == 0 && t.alphaYdecr < 0)) ? 0 : -1;
		t.betaBias = (t.betaXdecr > 0 || (t.betaXdecr == 0 && t.betaYdecr < 0)) ? 0 : -1;
		t.gammaBias = (gammaXdecr > 0 || (t.gammaXdecr == 0 && t.gammaYdecr < 0)) ? 0 : -1;

		/*
		compute AABB, using the integral part of its coordinates because it is tested against
		the bins coordinates, that are not expressed in fixed-point notation
		*/
		const int32_t xMin = std::min({ v0x, v1x, v2x }) >> 4;
		const int32_t xMax = std::max({ v0x, v1x, v2x }) >> 4;
		const int32_t yMin = std::min({ v0y, v1y, v2y }) >> 4;
		const int32_t yMax = std::max({ v0y, v1y, v2y }) >> 4;
		
		//find which bins are touched by the triangle
		for (size_t j = 0; j < m_binsCount; j++) {
			Bin& bin = m_bins[j];			
			
			if (intersectAABB(bin.xMin, bin.xMax, bin.yMin, bin.yMax, xMin, xMax, yMin, yMax)) {
				
				bin.addTriangle(i);

				if (m_activeBins.find(j) == m_activeBins.end()) {
					/*
					first triangle that is added to the bin. this is done in order to 
					keep track of which bin have some work to do. also, when multi-threading is 
					active this is necessary to start at most one task per bin.
					*/
					m_activeBins.insert(j);
#ifdef SOFTRP_MULTI_THREAD
					//start the task immediately while testing other bins and other triangles
					Bin* pBin = &bin;					
					bin.setDone(false);
					threadPool.addTask(
						[this, pBin, verticesPtr, instance]() {
							rasterizeBin(*pBin, verticesPtr, instance);
						}						
					);
#endif
				}
			}
		}
	}

#ifdef SOFTRP_MULTI_THREAD

	ThreadPool::Fence f = threadPool.addFence();

	//notify that no more triangles will be added to the lists
	for (size_t binIndex : m_activeBins)
		m_bins[binIndex].setDone(true);

	m_activeBins.clear();

	return f;
#else
	for (size_t binIndex : m_activeBins)
		rasterizeBin(m_bins[binIndex], verticesPtr, instance);

#endif
}

BinRasterizer::Bin::Bin(Bin&& bin) {	
#ifdef SOFTRP_MULTI_THREAD
	//TODO: this shouldn't be necessary
	std::lock_guard<std::mutex> lock{ mutex };
	done = bin.done;
#endif
	xMin = bin.xMin;
	yMin = bin.yMin;
	xMax = bin.xMax;
	yMax = bin.yMax;
	triangleQueue = std::move(bin.triangleQueue);
}

void BinRasterizer::Bin::addTriangle(size_t i) {
#ifdef SOFTRP_MULTI_THREAD
{
	std::lock_guard<std::mutex> lock{ mutex };
#endif
	triangleQueue.push(i);
#ifdef SOFTRP_MULTI_THREAD
}
	existsNext.notify_one();
#endif
}

#ifdef SOFTRP_MULTI_THREAD
void BinRasterizer::Bin::setDone(bool _done) {	
	{
		std::lock_guard<std::mutex> lock{ mutex };
		done = _done;
	}
	if (_done)
		existsNext.notify_one();
}
#endif

bool BinRasterizer::Bin::hasNext() {
#ifdef SOFTRP_MULTI_THREAD
	std::unique_lock<std::mutex> lock{ mutex };

	if (done && triangleQueue.size() == 0)
		return false;

	while (triangleQueue.size() == 0) {
		existsNext.wait(lock);
		if (done && triangleQueue.size() == 0)
			return false;
	}
#else
	if (triangleQueue.size() == 0)
		return false;
#endif
	return true;
}

size_t BinRasterizer::Bin::getNext() {
#ifdef SOFTRP_MULTI_THREAD
	std::unique_lock<std::mutex> lock{ mutex };
#endif
	size_t next = triangleQueue.front();
	triangleQueue.pop();	
	return next;
}

#ifndef SOFTRP_USE_SIMD
void SoftRP::BinRasterizer::rasterizeBin(Bin& bin, const std::vector<Vertex>* vertices, size_t instance) {

	Vertex vertexData2{};
	Vertex vertexData3{};

	PSExecutionContext execContext{};
	Vertex& interpolated0 = execContext.interpolated[0];
	Vertex& interpolated1 = execContext.interpolated[1];
	Vertex& interpolated2 = execContext.interpolated[2];
	Vertex& interpolated3 = execContext.interpolated[3];

	const int32_t xMin = bin.xMin;
	const int32_t xMax = bin.xMax;
	const int32_t yMin = bin.yMin;
	const int32_t yMax = bin.yMax;

	const int32_t renderTargetWidthMinusOne = static_cast<int32_t>(m_renderTargetWidth) - 1;
	const int32_t renderTargetHeightMinusOne = static_cast<int32_t>(m_renderTargetHeight) - 1;

	while (bin.hasNext()) {

		const size_t i = bin.getNext();
		const Triangle& t = m_triangles[i];

		const TransformedVertex& v0 = m_transformedVertices[t.i0];
		const TransformedVertex& v1 = m_transformedVertices[t.i1];
		const TransformedVertex& v2 = m_transformedVertices[t.i2];

		//evaluate edge functions at (xMin, yMin)		
		const int32_t fixedXMin = xMin << 4;
		const int32_t fixedYMin = yMin << 4;
		int32_t alpha0 = t.alpha0 + fixedXMin*t.alphaXdecr + fixedYMin*t.alphaYdecr;
		int32_t beta0 = t.beta0 + t.betaXdecr*fixedXMin + t.betaYdecr*fixedYMin;
		int32_t gamma0 = t.gamma0 + t.gammaXdecr*fixedXMin + t.gammaYdecr*fixedYMin;

		//compute decrements corresponding to a decrement of 1 in the integral part
		const int32_t alphaXdecr = t.alphaXdecr << 4;
		const int32_t alphaYdecr = t.alphaYdecr << 4;
		const int32_t betaXdecr = t.betaXdecr << 4;
		const int32_t betaYdecr = t.betaYdecr << 4;
		const int32_t gammaXdecr = t.gammaXdecr << 4;
		const int32_t gammaYdecr = t.gammaYdecr << 4;

		//decrements for movement 2 pixels wide
		const int32_t doubledAlphaXdecr = alphaXdecr << 1;
		const int32_t doubledAlphaYdecr = alphaYdecr << 1;
		const int32_t doubledBetaXdecr = betaXdecr << 1;
		const int32_t doubledBetaYdecr = betaYdecr << 1;
		const int32_t doubledGammaXdecr = gammaXdecr << 1;
		const int32_t doubledGammaYdecr = gammaYdecr << 1;

		//process a 2x2 pixel block at the time
		for (int32_t y = yMin; y < yMax; y += 2,
									 alpha0 += doubledAlphaYdecr,
									 beta0 += doubledBetaYdecr,
									 gamma0 += doubledGammaYdecr) {

			//edge functions for all pixels in the block		
			const int32_t alpha0PlusXdecr = alpha0 + alphaXdecr;			
			const int32_t beta0PlusXdecr = beta0 + betaXdecr;			
			const int32_t gamma0PlusXdecr = gamma0 + gammaXdecr;			
			int32_t alpha[4]{ alpha0PlusXdecr + alphaYdecr, alpha0 + alphaYdecr, alpha0PlusXdecr, alpha0 };
			int32_t beta[4]{ beta0PlusXdecr + betaYdecr,  beta0 + betaYdecr, beta0PlusXdecr, beta0};
			int32_t gamma[4]{ gamma0PlusXdecr + gammaYdecr, gamma0 + gammaYdecr, gamma0PlusXdecr, gamma0};

			for (int32_t x = xMin; x < xMax; x += 2) {

				const int32_t xPositions[4]{ x + 1, x , x + 1, x };
				const int32_t yPositions[4]{ y + 1 , y + 1 , y, y };

				//alpha, beta and gamma in floating-point notation
				float a[4];
				float b[4];
				float c[4];

				int32_t writeMask = 0;

				for (unsigned int k = 0; k < 4; k++) {

					/*
					Top-Left fill convention:
					Testing mask >=0 is equivalent to ((alpha + alphaBias) >= 0 && (beta + betaBias) >= 0 && (gamma + gammaBias) >= 0).
					This is because the sign bit of mask is 1 (negative) if any of the terms being ORed is.
					*/
					const int32_t mask = (alpha[k] + t.alphaBias) | (beta[k] + t.betaBias) | (gamma[k] + t.gammaBias);

					/*
					Inside render target test:
					x >= width iff x + 1 > width iff width - 1 < x iff (width - 1) - x < 0
					y >= height iff y + 1 > height iff height - 1 < y iff (height - 1) - y < 0
					So, using the same trick used for mask (sign bit), boundMask is negative if x >= width || y >= height
					*/
					const int32_t boundMask = 
						(renderTargetWidthMinusOne - xPositions[k]) | (renderTargetHeightMinusOne - yPositions[k]);

					a[k] = fixedToFloat<4>(alpha[k]);
					b[k] = fixedToFloat<4>(beta[k]);
					c[k] = fixedToFloat<4>(gamma[k]);
										
					if ((mask | boundMask) >= 0) {
						/*
						compute depth by linearly interpolate the vertices' depths
						Let A, B and C be the barycentric coordinates of the pixel, then:
						depth = A*depth0 + B*depth1 + C*depth2
						because A = alpha/(2*area), B = beta/(2*area) and C = gamma/(2*area), then:
						depth = (alpha*depth0 + beta*depth1 + gamma*depth2)/(2*area)
						*/
						const float depth = (a[k]*v0.position[2] + b[k]*v1.position[2] + c[k]*v2.position[2]) / t.twiceArea;
						if (depthTest(yPositions[k], xPositions[k], depth))
							writeMask |= (1 << k);
					}					
				}

				if (writeMask != 0) {

					//at least one pixel is inside the triangle and passed the depth test
					
					for (unsigned int k = 0; k < 4; k++) {

						/*
						interpolate vertices.
						to achieve perspective correct interpolation, a vertex field f is computed as follows:
						
						let:
						A, B and C the barycentric coordinates of the pixel;
						f0, f1 and f2 the value of the field at the triangle's vertices;
						w0, w1 and w2 the w coordinates of the triangle's vertices in Clip space (before perspective division);
						
						f = (A*f0/w0 + B*f1/w1 + C*f2/w2)/(A/w0 + B/w1 + C/w2)

						because A = alpha/(2*area), B = beta/(2*area) and C = gamma/(2*area), then:
						let A1 = alpha/w0, B1 = beta/w1, C1 = gamma/w2

						f = [(A1*f0 + B1*f1 + C1*f2)/(2*area)] / [(A1 + B1 + C1) / (2*area)]
						f = (A1*f0 + B1*f1 + C1*f2) / (A1 + B1 + C1)
						*/

						const float alphaOnW0 = v0.invW * a[k];
						const float betaOnW1 = v1.invW * b[k];
						const float gammaOnW2 = v2.invW * c[k];

						Vertex& interpolated = execContext.interpolated[k];

						interpolated = (*vertices)[t.i0];
						vertexData2 = (*vertices)[t.i1];
						vertexData3 = (*vertices)[t.i2];

						interpolated.scaleVertexData(alphaOnW0);
						vertexData2.scaleVertexData(betaOnW1);
						vertexData3.scaleVertexData(gammaOnW2);

						interpolated.addVertexData(vertexData2);
						interpolated.addVertexData(vertexData3);
						interpolated.scaleVertexData(1.0f / (alphaOnW0 + betaOnW1 + gammaOnW2));
					}

					//execute pixel shader
					execContext.mask = writeMask;
					Math::Vector4 outColors[4];
					(*pixelShader())(*shaderContext(), execContext, instance, outColors);

					//write pixels that are found to be inside and passed the depth test
					if ((writeMask & 0x1) != 0)
						writePixel(static_cast<unsigned int>(yPositions[0]), static_cast<unsigned int>(xPositions[0]), outColors[0]);
					if ((writeMask & 0x2) != 0)
						writePixel(static_cast<unsigned int>(yPositions[1]), static_cast<unsigned int>(xPositions[1]), outColors[1]);
					if ((writeMask & 0x4) != 0)
						writePixel(static_cast<unsigned int>(yPositions[2]), static_cast<unsigned int>(xPositions[2]), outColors[2]);
					if ((writeMask & 0x8) != 0)
						writePixel(static_cast<unsigned int>(yPositions[3]), static_cast<unsigned int>(xPositions[3]), outColors[3]);
				}

				for (unsigned int k = 0; k < 4; k++) {
					alpha[k] += doubledAlphaXdecr;
					beta[k] += doubledBetaXdecr;
					gamma[k] += doubledGammaXdecr;
				}
			}
		}
	}
}

#else
/*helper functions forward declarations*/
inline static int32_t depthTest(DepthBuffer* depthBuffer,
							const int32_t* i, const int32_t* j,
							const __m128 compare, const __m128i mask);

template<int32_t fractionalSize>
static __m128 fixedToFloat(__m128i fixed);
template<>
static __m128 fixedToFloat<4>(__m128i fixed);

void SoftRP::BinRasterizer::rasterizeBin(Bin& bin, const std::vector<Vertex>* vertices, size_t instance) {

	//the implementation follows the non-SIMD version. refer to it for details.

	static_assert((TILE_WIDTH % 2 == 0) && (TILE_HEIGHT % 2 == 0), "Invalid tile size, it must be a multiple of 2.");

	Vertex vertexData1{};
	Vertex vertexData2{};
	Vertex vertexData3{};

	PSExecutionContext execContext{};
	Vertex& interpolated0 = execContext.interpolated[0];
	Vertex& interpolated1 = execContext.interpolated[1];
	Vertex& interpolated2 = execContext.interpolated[2];
	Vertex& interpolated3 = execContext.interpolated[3];

	const int32_t xMin = bin.xMin;
	const int32_t xMax = bin.xMax;
	const int32_t yMin = bin.yMin;
	const int32_t yMax = bin.yMax;

	const int32_t renderTargetWidth = static_cast<int32_t>(m_renderTargetWidth);
	const int32_t renderTargetHeight = static_cast<int32_t>(m_renderTargetHeight);

	while (bin.hasNext()) {

		size_t i = bin.getNext();
		const Triangle& t = m_triangles[i];

		const TransformedVertex& v0 = m_transformedVertices[t.i0];
		const TransformedVertex& v1 = m_transformedVertices[t.i1];
		const TransformedVertex& v2 = m_transformedVertices[t.i2];

		const int32_t fixedXMin = xMin << 4;
		const int32_t fixedYMin = yMin << 4;
		int32_t alpha0 = t.alpha0 + fixedXMin*t.alphaXdecr + fixedYMin*t.alphaYdecr;
		int32_t beta0 = t.beta0 + t.betaXdecr*fixedXMin + t.betaYdecr*fixedYMin;
		int32_t gamma0 = t.gamma0 + t.gammaXdecr*fixedXMin + t.gammaYdecr*fixedYMin;

		const int32_t alphaXdecr = t.alphaXdecr << 4;
		const int32_t alphaYdecr = t.alphaYdecr << 4;
		const int32_t betaXdecr = t.betaXdecr << 4;
		const int32_t betaYdecr = t.betaYdecr << 4;
		const int32_t gammaXdecr = t.gammaXdecr << 4;
		const int32_t gammaYdecr = t.gammaYdecr << 4;

		const __m128i alphaBiasVec = _mm_set1_epi32(t.alphaBias);
		const __m128i betaBiasVec = _mm_set1_epi32(t.betaBias);
		const __m128i gammaBiasVec = _mm_set1_epi32(t.gammaBias);

		const int32_t alpha0PlusXdecr = alpha0 + alphaXdecr;
		__m128i alpha0Vec = _mm_set_epi32(alpha0, alpha0PlusXdecr, alpha0 + alphaYdecr, alpha0PlusXdecr + alphaYdecr);
		const int32_t beta0PlusXdecr = beta0 + betaXdecr;
		__m128i beta0Vec = _mm_set_epi32(beta0, beta0PlusXdecr, beta0 + betaYdecr, beta0PlusXdecr + betaYdecr);
		const int32_t gamma0PlusXdecr = gamma0 + gammaXdecr;
		__m128i gamma0Vec = _mm_set_epi32(gamma0, gamma0PlusXdecr, gamma0 + gammaYdecr, gamma0PlusXdecr + gammaYdecr);

		const __m128i alphaXdecrVec = _mm_set1_epi32(alphaXdecr << 1);
		const __m128i alphaYdecrVec = _mm_set1_epi32(alphaYdecr << 1);
		const __m128i betaXdecrVec = _mm_set1_epi32(betaXdecr << 1);
		const __m128i betaYdecrVec = _mm_set1_epi32(betaYdecr << 1);
		const __m128i gammaXdecrVec = _mm_set1_epi32(gammaXdecr << 1);
		const __m128i gammaYdecrVec = _mm_set1_epi32(gammaYdecr << 1);

		const __m128i inverseTest = _mm_set1_epi32(0);
		const __m128 twiceArea = _mm_set_ps1(t.twiceArea);

		const __m128 v0Z = _mm_set_ps1(v0.position[2]);
		const __m128 v1Z = _mm_set_ps1(v1.position[2]);
		const __m128 v2Z = _mm_set_ps1(v2.position[2]);
		const __m128 invW0 = _mm_set_ps1(v0.invW);
		const __m128 invW1 = _mm_set_ps1(v1.invW);
		const __m128 invW2 = _mm_set_ps1(v2.invW);

		vertexData1 = (*vertices)[t.i0];
		vertexData2 = (*vertices)[t.i1];
		vertexData3 = (*vertices)[t.i2];

		interpolated0 = vertexData1;
		interpolated1 = vertexData1;
		interpolated2 = vertexData1;
		interpolated3 = vertexData1;

		for (int32_t y = yMin; y < yMax; y += 2,
									 alpha0Vec = _mm_add_epi32(alpha0Vec, alphaYdecrVec),
									 beta0Vec = _mm_add_epi32(beta0Vec, betaYdecrVec),
									 gamma0Vec = _mm_add_epi32(gamma0Vec, gammaYdecrVec)) {

			__m128i alpha = alpha0Vec;
			__m128i beta = beta0Vec;
			__m128i gamma = gamma0Vec;

			for (int32_t x = xMin; x < xMax; x += 2,
										 alpha = _mm_add_epi32(alpha, alphaXdecrVec),
										 beta = _mm_add_epi32(beta, betaXdecrVec),
										 gamma = _mm_add_epi32(gamma, gammaXdecrVec)) {

				const __m128i mask = _mm_or_si128(_mm_or_si128(_mm_add_epi32(alpha, alphaBiasVec), _mm_add_epi32(beta, betaBiasVec)),
												  _mm_add_epi32(gamma, gammaBiasVec));
				// for each 32-bit integer : 0xFFFFFFFF if test failed (mask < 0), 0 otherwise.
				__m128i compInside = _mm_cmplt_epi32(mask, inverseTest);
				//create mask with the most significant bit of all 8-bit integers packed in argument
				const int test = _mm_movemask_epi8(compInside); 

				if (test == 0xFFFF)//all failed?
					continue;

				const int32_t x0 = x + 1;
				const int32_t y0 = y + 1;
				const int32_t x1 = x;
				const int32_t y1 = y0;
				const int32_t x2 = x0;
				const int32_t y2 = y;
				const int32_t x3 = x;
				const int32_t y3 = y;
																
				const __m128i xPositionsVec = _mm_set_epi32(x3, x2, x1, x0);
				const __m128i yPositionsVec = _mm_set_epi32(y3, y2, y1, y0);
				const __m128i widthBoundMask = _mm_set1_epi32(renderTargetWidth);
				const __m128i heightBoundMask = _mm_set1_epi32(renderTargetHeight);

				// for each x,y coordinate : 0xFFFFFFFF if inside rendertarget (x < width && y < height), 0 otherwise.
				const __m128i boundMask =
					_mm_and_si128(
						_mm_cmplt_epi32(xPositionsVec, widthBoundMask),
						_mm_cmplt_epi32(yPositionsVec, heightBoundMask));

				//since compInside == 0 means success, then write pixel iff ((~compInside == 0xFFFFFFFF) & boundMask ) == 0xFFFFFFFF
				__m128i comp = _mm_andnot_si128(compInside, boundMask);

				const __m128 a = fixedToFloat<4>(alpha);
				const __m128 b = fixedToFloat<4>(beta);
				const __m128 c = fixedToFloat<4>(gamma);

				const __m128 depthV0 = _mm_mul_ps(v0Z, a);
				const __m128 depthV1 = _mm_mul_ps(v1Z, b);
				const __m128 depthV2 = _mm_mul_ps(v2Z, c);
				const __m128 depth = _mm_div_ps(_mm_add_ps(_mm_add_ps(depthV0, depthV1), depthV2), twiceArea);

				const int32_t xPositions[4]{ x0, x1, x2, x3	};
				const int32_t yPositions[4]{ y0, y1, y2, y3	};
				const int32_t depthTestRes = ::depthTest(depthBuffer(), yPositions, xPositions, depth, comp);

				if (depthTestRes == 0x0)//all failed?
					continue;

				const __m128 alphaOnW0 = _mm_mul_ps(invW0, a);
				const __m128 betaOnW1 = _mm_mul_ps(invW1, b);
				const __m128 gammaOnW2 = _mm_mul_ps(invW2, c);

				const __m128 onWSumInv = _mm_rcp_ps(_mm_add_ps(_mm_add_ps(alphaOnW0, betaOnW1), gammaOnW2));

				const __m128 splatOnW00 = _mm_set_ps1(alphaOnW0.m128_f32[0]);
				const __m128 splatOnW01 = _mm_set_ps1(alphaOnW0.m128_f32[1]);
				const __m128 splatOnW02 = _mm_set_ps1(alphaOnW0.m128_f32[2]);
				const __m128 splatOnW03 = _mm_set_ps1(alphaOnW0.m128_f32[3]);

				const __m128 splatOnW10 = _mm_set_ps1(betaOnW1.m128_f32[0]);
				const __m128 splatOnW11 = _mm_set_ps1(betaOnW1.m128_f32[1]);
				const __m128 splatOnW12 = _mm_set_ps1(betaOnW1.m128_f32[2]);
				const __m128 splatOnW13 = _mm_set_ps1(betaOnW1.m128_f32[3]);

				const __m128 splatOnW20 = _mm_set_ps1(gammaOnW2.m128_f32[0]);
				const __m128 splatOnW21 = _mm_set_ps1(gammaOnW2.m128_f32[1]);
				const __m128 splatOnW22 = _mm_set_ps1(gammaOnW2.m128_f32[2]);
				const __m128 splatOnW23 = _mm_set_ps1(gammaOnW2.m128_f32[3]);

				const __m256 splatOnW0_01 = _mm256_set_m128(splatOnW00, splatOnW01);
				const __m256 splatOnW0_23 = _mm256_set_m128(splatOnW02, splatOnW03);
				const __m256 splatOnW1_01 = _mm256_set_m128(splatOnW10, splatOnW11);
				const __m256 splatOnW1_23 = _mm256_set_m128(splatOnW12, splatOnW13);
				const __m256 splatOnW2_01 = _mm256_set_m128(splatOnW20, splatOnW21);
				const __m256 splatOnW2_23 = _mm256_set_m128(splatOnW22, splatOnW23);

				const __m128 onWSumInv0 = _mm_set_ps1(onWSumInv.m128_f32[0]);
				const __m128 onWSumInv1 = _mm_set_ps1(onWSumInv.m128_f32[1]);
				const __m128 onWSumInv2 = _mm_set_ps1(onWSumInv.m128_f32[2]);
				const __m128 onWSumInv3 = _mm_set_ps1(onWSumInv.m128_f32[3]);

				const __m256 doubledOnWSumInv01 = _mm256_set_m128(onWSumInv0, onWSumInv1);
				const __m256 doubledOnWSumInv23 = _mm256_set_m128(onWSumInv2, onWSumInv3);

				const size_t fieldCount = vertexData1.fieldCount();

				//interpolate vertices, excluding position
				for (size_t f = 1; f < fieldCount; f++) {
					const float* field1 = vertexData1.getField(f);
					const float* field2 = vertexData2.getField(f);
					const float* field3 = vertexData3.getField(f);

					const __m128 field1Vec = _mm_load_ps(field1);
					const __m128 field2Vec = _mm_load_ps(field2);
					const __m128 field3Vec = _mm_load_ps(field3);

					//process 2 adjacent pixels at the time
					const __m256 doubledField1 = _mm256_set_m128(field1Vec, field1Vec);
					const __m256 doubledField2 = _mm256_set_m128(field2Vec, field2Vec);
					const __m256 doubledField3 = _mm256_set_m128(field3Vec, field3Vec);

					__m256 scale1 = _mm256_mul_ps(doubledField1, splatOnW0_01);
					__m256 scale2 = _mm256_mul_ps(doubledField2, splatOnW1_01);
					__m256 scale3 = _mm256_mul_ps(doubledField3, splatOnW2_01);

					scale1 = _mm256_mul_ps(_mm256_add_ps(_mm256_add_ps(scale1, scale2), scale3), doubledOnWSumInv01);

					float* target0 = interpolated0.getField(f);
					float* target1 = interpolated1.getField(f);
					_mm256_storeu2_m128(target1, target0, scale1);

					scale1 = _mm256_mul_ps(doubledField1, splatOnW0_23);
					scale2 = _mm256_mul_ps(doubledField2, splatOnW1_23);
					scale3 = _mm256_mul_ps(doubledField3, splatOnW2_23);

					scale1 = _mm256_mul_ps(_mm256_add_ps(_mm256_add_ps(scale1, scale2), scale3), doubledOnWSumInv23);

					float* target2 = interpolated2.getField(f);
					float* target3 = interpolated3.getField(f);
					_mm256_storeu2_m128(target3, target2, scale1);
				}

				//execute pixel shader
				const PixelShader& ps = *pixelShader();
				execContext.mask = depthTestRes;
								
				Math::Vector4 outColors[4];
				ps(*shaderContext(), execContext, instance, outColors);

				//write pixels that are found to be inside and passed the depth test
				if ((depthTestRes & 0x1) != 0)
					writePixel(static_cast<unsigned int>(yPositions[0]), static_cast<unsigned int>(xPositions[0]), outColors[0]);
				if ((depthTestRes & 0x2) != 0)
					writePixel(static_cast<unsigned int>(yPositions[1]), static_cast<unsigned int>(xPositions[1]), outColors[1]);
				if ((depthTestRes & 0x4) != 0)
					writePixel(static_cast<unsigned int>(yPositions[2]), static_cast<unsigned int>(xPositions[2]), outColors[2]);
				if ((depthTestRes & 0x8) != 0)
					writePixel(static_cast<unsigned int>(yPositions[3]), static_cast<unsigned int>(xPositions[3]), outColors[3]);
			}
		}
	}
}
#endif

inline static bool intersectAABB(int32_t xMin1, int32_t xMax1, int32_t yMin1, int32_t yMax1, int32_t xMin2, int32_t xMax2, int32_t yMin2, int32_t yMax2) {
	if (xMin2 > xMax1)
		return false;

	if (xMin1 > xMax2)
		return false;

	if (yMin2 > yMax1)
		return false;

	if (yMin1 > yMax2)
		return false;

	return true;
}

template<int32_t fractionalSize>
inline static int32_t fixedFromFloat(float x) {
	throw std::runtime_error{ "Missing implementation" }:
}

inline static int iRound(float x) {
	return static_cast<int>(std::floor(x + 0.5f));
}

template<>
inline static int32_t fixedFromFloat<4>(float x) {
	return iRound(x * 16.0f);
}

template<int32_t fractionalSize>
inline static float fixedToFloat(int32_t fixed) {
	throw std::runtime_error{ "Missing implementation" }:
}

template<>
inline static float fixedToFloat<4>(int32_t fixed) {
	int32_t integralPart = fixed >> 4;
	fixed &= 0x0000000F;
	return static_cast<float>(integralPart) + (static_cast<float>(fixed) / 16.0f);
}

#ifdef SOFTRP_USE_SIMD
template<int32_t fractionalSize>
inline static __m128 fixedToFloat(__m128i fixed) {
	throw std::runtime_error{ "Missing implementation" }:
}

template<>
inline static __m128 fixedToFloat<4>(__m128i fixed) {
	const __m128i integralPart = _mm_set_epi32(fixed.m128i_i32[3] >> 4, fixed.m128i_i32[2] >> 4, fixed.m128i_i32[1] >> 4, fixed.m128i_i32[0] >> 4);
	//const __m128i integralPart = _mm_srli_epi32(fixed, 4); Can't understand why this line is not equivalent to the one above.
	const __m128i fractionalMask = _mm_set1_epi32(0x0000000F);
	fixed = _mm_and_si128(fixed, fractionalMask);
	const __m128 fractional = _mm_cvtepi32_ps(fixed);
	const __m128 divisor = _mm_set_ps1(16.0f);
	return _mm_add_ps(_mm_cvtepi32_ps(integralPart), _mm_div_ps(fractional, divisor));
}

inline static int32_t depthTest(DepthBuffer* depthBuffer, const int32_t* i, const int32_t* j, const __m128 compare, const __m128i mask) {

	int32_t curr = 1;
	int32_t res = 0;

	for (unsigned int k = 0; k < 4; k++, curr <<= 1) {
		if (mask.m128i_i32[k] == 0)
			continue;
		const unsigned int row = static_cast<unsigned int>(i[k]);
		const unsigned int column = static_cast<unsigned int>(j[k]);
		const float currDepth = depthBuffer->get(row, column);
		const float compareVal = compare.m128_f32[k];
		if (currDepth > compareVal) {
			depthBuffer->set(row, column, compareVal);
			res |= curr;
		}
	}

	return res;
}

#endif