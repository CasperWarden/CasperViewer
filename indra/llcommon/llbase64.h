/** 
 * @file llbase64.h
 * @brief Wrapper for apr base64 encoding that returns a std::string
 * @author James Cook
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LLBASE64_H
#define LLBASE64_h

class LL_COMMON_API LLBase64
{
public:
	/**
	 * Encodes a binary array into a string.
	 * @param[in] input The buffer to encode.
	 * @param[in] input_size The length of data to encode.
	 * @return A base64 string representing the encoded data.
	 */
	static std::string encode(const U8* input, size_t input_size);
	
	/**
	 * Decodes a string into a binary vector.
	 * @param[in] input The base64 string to decode.
	 * @return A vector containing the decoded binary data.
	 */
	static std::vector<U8> decode(std::string input);
};

#endif
