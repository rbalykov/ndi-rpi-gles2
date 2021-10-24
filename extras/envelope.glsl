
mat3 ycbcr_to_rgb_mat = mat3(1.16414, -0.0011, 1.7923, 1.16390, -0.2131, -0.5342, 1.16660, 2.1131, -0.0001);
vec3 ycbcr_to_rgb_vec = vec3(-0.9726, 0.3018, -1.1342);

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{

    float scale_x = iResolution.x/pPixels;
    float scale_y = iResolution.y/pHeight;
    float scale = min(scale_x, scale_y);
    float x_limit = pPixels/pPairs/2.0;


    if (scale_x > scale_y) { scale_x = scale_x/scale_y;}
    else {scale_y = scale_y/scale_x;}

//    vec2 uv = vec2( (((pPixels/pPairs)/2.0) * fragCoord.x/iResolution.x) * scale_x,  1.0-fragCoord.y/pHeight * scale_y);

    vec2 uv = vec2( (fragCoord.x - iResolution.x/2.0)/pPixels/scale + x_limit/2.0, (iResolution.y/2.0 - fragCoord.y + 1.0)/pHeight/scale + 0.5 );
    

  if ((uv.x > 0.0 ) && (uv.x < x_limit) && (uv.y > 0.0) && (uv.y < 1.0))
{
    float tex_x  = uv.x * pPairs;
    tex_x = tex_x - floor(tex_x);
    vec3 col_y_cbcr;// = (0.0,0.0,0.0);
    if (tex_x < 0.5)
	col_y_cbcr = texture2D(pVideo, uv.xy).grb;
    else
	col_y_cbcr = texture2D(pVideo, uv.xy).arb;

    vec3 col_rgb = vec3( dot( ycbcr_to_rgb_mat[0], col_y_cbcr ), dot( ycbcr_to_rgb_mat[1], col_y_cbcr ), dot( ycbcr_to_rgb_mat[2], col_y_cbcr ) ) + ycbcr_to_rgb_vec;
    fragColor = vec4(col_rgb, 1.0);
}
else {fragColor = vec4(0.0, 0.0, 0.0, 0.0);}
}
