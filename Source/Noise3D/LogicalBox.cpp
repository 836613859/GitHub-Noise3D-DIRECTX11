
/*******************************************************

								cpp ��Logical Box

********************************************************/

#include "Noise3D.h"

using namespace Noise3D;
using namespace Noise3D::D3D;

NVECTOR3 Noise3D::LogicalBox::ComputeNormal(NOISE_BOX_FACET facet)
{
	switch (facet)
	{
	case NOISE_BOX_FACET::POS_X: return NVECTOR3(1.0f, 0, 0);
	case NOISE_BOX_FACET::NEG_X: return NVECTOR3(-1.0f, 0, 0);
	case NOISE_BOX_FACET::POS_Y: return NVECTOR3(0, 1.0f, 0);
	case NOISE_BOX_FACET::NEG_Y: return NVECTOR3(0, -1.0f, 0);
	case NOISE_BOX_FACET::POS_Z: return NVECTOR3(0, 0, 1.0f);
	case NOISE_BOX_FACET::NEG_Z: return NVECTOR3(0, 0, 1.0f);
	}
	return NVECTOR3(0, 0, 0);
}
