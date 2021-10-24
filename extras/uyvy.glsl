varying vec4 tx_coord;

uniform sampler2D video_Y, video_CbCr;

mat3 ycbcr_to_rgb_mat = mat3(1.16414, -0.0011, 1.7923, 1.16390, -0.2131, -0.5342, 1.16660, 2.1131, -0.0001);
vec3 ycbcr_to_rgb_vec = vec3(-0.9726, 0.3018, -1.1342);

void main()
{
vec3 col_y_cbcr = vec3(texture2D(video_Y, tx_coord.xy).r, texture2D(video_CbCr, tx_coord.xy).rg);
vec3 col_rgb = vec3( dot( ycbcr_to_rgb_mat[0], col_y_cbcr ), dot( ycbcr_to_rgb_mat[1], col_y_cbcr ), dot( ycbcr_to_rgb_mat[2], col_y_cbcr ) ) + ycbcr_to_rgb_vec;
gl_FragData[0] = vec4( col_rgb, 1.0 );
}

