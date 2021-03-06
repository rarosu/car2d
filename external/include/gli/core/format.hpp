///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Image (gli.g-truc.net)
///
/// Copyright (c) 2008 - 2012 G-Truc Creation (www.g-truc.net)
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
/// @file gli/core/format.hpp
/// @date 2012-10-16 / 2015-06-16
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace gli
{
	enum format
	{
		// unorm formats
		FORMAT_R8_UNORM = 0, FORMAT_FIRST = FORMAT_R8_UNORM,
		FORMAT_RG8_UNORM,
		FORMAT_RGB8_UNORM,
		FORMAT_RGBA8_UNORM,

		FORMAT_R16_UNORM,
		FORMAT_RG16_UNORM,
		FORMAT_RGB16_UNORM,
		FORMAT_RGBA16_UNORM,

		// snorm formats
		FORMAT_R8_SNORM,
		FORMAT_RG8_SNORM,
		FORMAT_RGB8_SNORM,
		FORMAT_RGBA8_SNORM,

		FORMAT_R16_SNORM,
		FORMAT_RG16_SNORM,
		FORMAT_RGB16_SNORM,
		FORMAT_RGBA16_SNORM,

		// Unsigned integer formats
		FORMAT_R8_UINT,
		FORMAT_RG8_UINT,
		FORMAT_RGB8_UINT,
		FORMAT_RGBA8_UINT,

		FORMAT_R16_UINT,
		FORMAT_RG16_UINT,
		FORMAT_RGB16_UINT,
		FORMAT_RGBA16_UINT,

		FORMAT_R32_UINT,
		FORMAT_RG32_UINT,
		FORMAT_RGB32_UINT,
		FORMAT_RGBA32_UINT,

		// Signed integer formats
		FORMAT_R8_SINT,
		FORMAT_RG8_SINT,
		FORMAT_RGB8_SINT,
		FORMAT_RGBA8_SINT,

		FORMAT_R16_SINT,
		FORMAT_RG16_SINT,
		FORMAT_RGB16_SINT,
		FORMAT_RGBA16_SINT,

		FORMAT_R32_SINT,
		FORMAT_RG32_SINT,
		FORMAT_RGB32_SINT,
		FORMAT_RGBA32_SINT,

		// Floating formats
		FORMAT_R16_SFLOAT,
		FORMAT_RG16_SFLOAT,
		FORMAT_RGB16_SFLOAT,
		FORMAT_RGBA16_SFLOAT,

		FORMAT_R32_SFLOAT,
		FORMAT_RG32_SFLOAT,
		FORMAT_RGB32_SFLOAT,
		FORMAT_RGBA32_SFLOAT,

		// sRGB formats
		FORMAT_R8_SRGB,
		FORMAT_RG8_SRGB,
		FORMAT_RGB8_SRGB,
		FORMAT_RGBA8_SRGB,

		// Packed formats
		FORMAT_RGB10A2_UNORM,
		FORMAT_RGB10A2_UINT,
		FORMAT_RGB9E5_UFLOAT,
		FORMAT_RG11B10_UFLOAT,
		FORMAT_RG3B2_UNORM,
		FORMAT_R5G6B5_UNORM,
		FORMAT_RGB5A1_UNORM,
		FORMAT_RGBA4_UNORM,

		// Swizzle formats
		FORMAT_BGRX8_UNORM,
		FORMAT_BGRA8_UNORM,
		FORMAT_BGRX8_SRGB,
		FORMAT_BGRA8_SRGB,

		// Luminance Alpha formats
		FORMAT_L8_UNORM,
		FORMAT_A8_UNORM,
		FORMAT_LA8_UNORM,
		FORMAT_L16_UNORM,
		FORMAT_A16_UNORM,
		FORMAT_LA16_UNORM,

		// Depth formats
		FORMAT_D16_UNORM,
		FORMAT_D24_UNORM,
		FORMAT_D24S8_UNORM,
		FORMAT_D32_UFLOAT,
		FORMAT_D32_UFLOAT_S8_UNORM,

		// Compressed formats
		FORMAT_RGB_DXT1_UNORM, FORMAT_COMPRESSED_FIRST = FORMAT_RGB_DXT1_UNORM,
		FORMAT_RGBA_DXT1_UNORM,
		FORMAT_RGBA_DXT3_UNORM,
		FORMAT_RGBA_DXT5_UNORM,
		FORMAT_R_ATI1N_UNORM,
		FORMAT_R_ATI1N_SNORM,
		FORMAT_RG_ATI2N_UNORM,
		FORMAT_RG_ATI2N_SNORM,
		FORMAT_RGB_BP_UFLOAT,
		FORMAT_RGB_BP_SFLOAT,
		FORMAT_RGB_BP_UNORM,
		FORMAT_RGB_PVRTC_8X8_UNORM,
		FORMAT_RGB_PVRTC_16X8_UNORM,
		FORMAT_RGBA_PVRTC_8X8_UNORM,
		FORMAT_RGBA_PVRTC_16X8_UNORM,
		FORMAT_RGBA_PVRTC2_8X8_UNORM,
		FORMAT_RGBA_PVRTC2_16X8_UNORM,
		FORMAT_RGB_ATC_UNORM,
		FORMAT_RGBA_ATC_EXPLICIT_UNORM,
		FORMAT_RGBA_ATC_INTERPOLATED_UNORM,
		FORMAT_RGB_ETC_UNORM,
		FORMAT_RGBA_ETC2_PUNCHTHROUGH_UNORM,
		FORMAT_RGBA_ETC2_UNORM,
		FORMAT_R11_EAC_UNORM,
		FORMAT_R11_EAC_SNORM,
		FORMAT_RG11_EAC_UNORM,
		FORMAT_RG11_EAC_SNORM,
		FORMAT_RGBA_ASTC_4X4_UNORM,
		FORMAT_RGBA_ASTC_5X4_UNORM,
		FORMAT_RGBA_ASTC_5X5_UNORM,
		FORMAT_RGBA_ASTC_6X5_UNORM,
		FORMAT_RGBA_ASTC_6X6_UNORM,
		FORMAT_RGBA_ASTC_8X5_UNORM,
		FORMAT_RGBA_ASTC_8X6_UNORM,
		FORMAT_RGBA_ASTC_8X8_UNORM,
		FORMAT_RGBA_ASTC_10X5_UNORM,
		FORMAT_RGBA_ASTC_10X6_UNORM,
		FORMAT_RGBA_ASTC_10X8_UNORM,
		FORMAT_RGBA_ASTC_10X10_UNORM,
		FORMAT_RGBA_ASTC_12X10_UNORM,
		FORMAT_RGBA_ASTC_12X12_UNORM,

		// Compressed sRGB formats
		FORMAT_RGB_DXT1_SRGB,
		FORMAT_RGBA_DXT1_SRGB,
		FORMAT_RGBA_DXT3_SRGB,
		FORMAT_RGBA_DXT5_SRGB,
		FORMAT_RGB_BP_SRGB,
		FORMAT_RGB_PVRTC_8X8_SRGB,
		FORMAT_RGB_PVRTC_16X8_SRGB,
		FORMAT_RGBA_PVRTC_8X8_SRGB,
		FORMAT_RGBA_PVRTC_16X8_SRGB,
		FORMAT_RGBA_PVRTC2_8X8_SRGB,
		FORMAT_RGBA_PVRTC2_16X8_SRGB,
		FORMAT_RGB_ETC_SRGB,
		FORMAT_RGBA_ETC2_PUNCHTHROUGH_SRGB,
		FORMAT_RGBA_ETC2_SRGB,
		FORMAT_RGBA_ASTC_4X4_SRGB,
		FORMAT_RGBA_ASTC_5X4_SRGB,
		FORMAT_RGBA_ASTC_5X5_SRGB,
		FORMAT_RGBA_ASTC_6X5_SRGB,
		FORMAT_RGBA_ASTC_6X6_SRGB,
		FORMAT_RGBA_ASTC_8X5_SRGB,
		FORMAT_RGBA_ASTC_8X6_SRGB,
		FORMAT_RGBA_ASTC_8X8_SRGB,
		FORMAT_RGBA_ASTC_10X5_SRGB,
		FORMAT_RGBA_ASTC_10X6_SRGB,
		FORMAT_RGBA_ASTC_10X8_SRGB,
		FORMAT_RGBA_ASTC_10X10_SRGB,
		FORMAT_RGBA_ASTC_12X10_SRGB,
		FORMAT_RGBA_ASTC_12X12_SRGB, FORMAT_COMPRESSED_LAST = FORMAT_RGBA_ASTC_12X12_SRGB, FORMAT_LAST = FORMAT_RGBA_ASTC_12X12_SRGB
	};

	enum
	{
		FORMAT_INVALID = -1,
		FORMAT_COUNT = FORMAT_LAST - FORMAT_FIRST + 1,
		FORMAT_COMPRESSED_COUNT = FORMAT_COMPRESSED_LAST - FORMAT_COMPRESSED_FIRST + 1
	};

	inline bool is_compressed(format const & Format)
	{
		return Format >= FORMAT_COMPRESSED_FIRST && Format <= FORMAT_COMPRESSED_LAST;
	}

	inline bool is_valid(format const & Format)
	{
		return Format >= FORMAT_FIRST && Format <= FORMAT_LAST;
	}

	std::uint32_t block_size(format const & Format);
	std::uint32_t block_dimensions_x(format const & Format);
	std::uint32_t block_dimensions_y(format const & Format);
	std::uint32_t block_dimensions_z(format const & Format);
	std::uint32_t component_count(format const & Format);
}//namespace gli

#include "format.inl"
