/*
 * VideoShader.h
 *
 *  Created on: 25 окт. 2021 г.
 *      Author: pi
 */

#ifndef SRC_VIDEOSHADER_H_
#define SRC_VIDEOSHADER_H_

#include <cstddef>
#include <GLES2/gl2.h>
#include <Processing.NDI.Advanced.h>

class VideoShader
{
public:
	VideoShader(const char* file);
	virtual ~VideoShader();
	GLuint id() { return m_program; };
	GLuint ok() { return m_ok; };

private:
	bool m_ok;
	GLuint m_program;
//	NDIlib_FourCC_video_type_e m_four_cc;

	int load_shader(const char *file);
	int create_program(const char *vs_src, const char *fs_src);
	int link_program(unsigned program);

	static const char fullscreen_vs[];
	static const char fullscreen_fs_head[];
	static const char fullscreen_fs_tail[];


};

#endif /* SRC_VIDEOSHADER_H_ */
