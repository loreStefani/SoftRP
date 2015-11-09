#ifndef SOFTRP_VIEWPORT_H_
#define SOFTRP_VIEWPORT_H_
#include "Matrix.h"
#include<stdexcept>
namespace SoftRP {
	
	/*
	Concrete data type which represents the transformation from NDC space to Screen space, a 3D space where 
	the third dimension is used for depth buffering.
	The transformation flips the y-axis in order to make the origin corresponds to the upper-left corner of the render target.	
	*/

	class ViewPort{
	public:
				
		/*
		ctor. Construct a ViewPort which transforms NDC space [-1,1]^3 to 
		Screen space [x, x + width]*[y + height, y]*[minZ, maxZ].
		Note the [y + height, y] is used to emphasize the fact that the transformation
		flips the y-axis.
		*/
		ViewPort(unsigned int width, unsigned int height, 
				 unsigned int x = 0, unsigned int y = 0, 
				 float minZ = 0.0f, float maxZ = 1.0f);

		~ViewPort() = default;
		
		//copy
		ViewPort(const ViewPort&);
		ViewPort& operator=(const ViewPort&);
		
		//move
		ViewPort(ViewPort&&) = default;		
		ViewPort& operator=(ViewPort&&) = default;

		/*	accessors	*/
		unsigned int getX() const;
		unsigned int getY() const;
		unsigned int getWidth()const;
		unsigned int getHeight()const;
		float getMinZ() const;
		float getMaxZ() const;		
		const Math::Matrix4& getTransform();
		const Math::Matrix4& getTransform() const;

		/*	setters	*/
		void setX(const unsigned int x);
		void setY(const unsigned int y);
		void setWidth(const unsigned int width);
		void setHeight(const unsigned int height);
		void setMinZ(const float minZ);
		void setMaxZ(const float maxZ);

	private:

		void copy(const ViewPort&);
		void updateTransform();

		bool m_needsUpdate{true};
		unsigned int m_x;
		unsigned int m_y;
		unsigned int m_width;
		unsigned int m_height;
		float m_minZ;
		float m_maxZ;		
		Math::Matrix4 m_transform;
	};
}
#include "ViewPortImpl.inl"
#endif
