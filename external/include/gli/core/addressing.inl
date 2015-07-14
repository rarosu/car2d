///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Image (gli.g-truc.net)
///
/// Copyright (c) 2008 - 2015 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref core
/// @file gli/core/addressing.inl
/// @date 2012-11-19 / 2012-11-19
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

namespace gli{
namespace detail
{
	inline storage::size_type imageAddressing
	(
		storage const & Storage,
		storage::size_type const & LayerOffset,
		storage::size_type const & FaceOffset,
		storage::size_type const & LevelOffset
	)
	{
		assert(LayerOffset < Storage.layers());
		assert(FaceOffset < Storage.faces());
		assert(LevelOffset < Storage.levels());

		storage::size_type LayerSize = Storage.layer_size(0, Storage.faces() - 1, 0, Storage.levels() - 1);
		storage::size_type FaceSize = Storage.face_size(0, Storage.levels() - 1);
		storage::size_type BaseOffset = LayerSize * LayerOffset + FaceSize * FaceOffset; 

		for(storage::size_type Level(0); Level < LevelOffset; ++Level)
			BaseOffset += Storage.level_size(Level);

		return BaseOffset;
	}

	inline storage::size_type texelLinearAdressing
	(
		storage::dim1_type const & Dimensions,
		storage::dim1_type const & TexelCoord
	)
	{
		assert(glm::all(glm::lessThan(TexelCoord, Dimensions)));

		return TexelCoord.x;
	}

	inline storage::size_type texelLinearAdressing
	(
		storage::dim2_type const & Dimensions,
		storage::dim2_type const & TexelCoord
	)
	{
		assert(TexelCoord.x < Dimensions.x);
		assert(TexelCoord.y < Dimensions.y);

		return TexelCoord.x + Dimensions.x * TexelCoord.y;
	}

	inline storage::size_type texelLinearAdressing
	(
		storage::dim3_type const & Dimensions,
		storage::dim3_type const & TexelCoord
	)
	{
		assert(TexelCoord.x < Dimensions.x);
		assert(TexelCoord.y < Dimensions.y);
		assert(TexelCoord.z < Dimensions.z);

		return TexelCoord.x + Dimensions.x * (TexelCoord.y + Dimensions.y * TexelCoord.z);
	}

	inline storage::size_type texelMortonAdressing
	(
		storage::dim1_type const & Dimensions,
		storage::dim1_type const & TexelCoord
	)
	{
		assert(TexelCoord.x < Dimensions.x);

		return TexelCoord.x;
	}

	inline storage::size_type texelMortonAdressing
	(
		storage::dim2_type const & Dimensions,
		storage::dim2_type const & TexelCoord
	)
	{
		assert(TexelCoord.x < Dimensions.x && TexelCoord.x < std::numeric_limits<std::uint32_t>::max());
		assert(TexelCoord.y < Dimensions.y && TexelCoord.y < std::numeric_limits<std::uint32_t>::max());

		glm::u32vec2 const Input(TexelCoord);

		return glm::bitfieldInterleave(Input.x, Input.y);
	}

	inline storage::size_type texelMortonAdressing
	(
		storage::dim3_type const & Dimensions,
		storage::dim3_type const & TexelCoord
	)
	{
		assert(TexelCoord.x < Dimensions.x);
		assert(TexelCoord.y < Dimensions.y);
		assert(TexelCoord.z < Dimensions.z);

		glm::u32vec3 const Input(TexelCoord);

		return glm::bitfieldInterleave(Input.x, Input.y, Input.z);
	}
}//namespace detail
}//namespace gli
