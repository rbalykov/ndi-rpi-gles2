
mat3 ycbcr_to_rgb_mat = mat3(1.16414, -0.0011, 1.7923, 1.16390, -0.2131, -0.5342, 1.16660, 2.1131, -0.0001);
vec3 ycbcr_to_rgb_vec = vec3(-0.9726, 0.3018, -1.1342);

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = vec2(((pPixels/pPairs)/2.0)*fragCoord.x/iResolution.x, 1.0-fragCoord.y/iResolution.y);

    float tex_x  = uv.x * pPairs;
    tex_x = tex_x - floor(tex_x);
    vec3 col_y_cbcr;// = (0.0,0.0,0.0);
    if (tex_x < 0.5)
    	col_y_cbcr = texture2D(pVideo, uv.xy).grb;
    else
    	col_y_cbcr = texture2D(pVideo, uv.xy).arb;

    // formula from newtek forum
    vec3 col_rgb = vec3( dot( ycbcr_to_rgb_mat[0], col_y_cbcr ), dot( ycbcr_to_rgb_mat[1], col_y_cbcr ), dot( ycbcr_to_rgb_mat[2], col_y_cbcr ) ) + ycbcr_to_rgb_vec;
    fragColor = vec4(col_rgb, 1.0);

    /*  
    // simpler formula taken somewhere on web
    float delta = 0.50;
    vec3 uyv = vec3(col_y_cbcr.brg);
    fragColor.r = uyv.g + 1.403*(uyv.r - delta);
    fragColor.g = uyv.g - 0.714*(uyv.r - delta) - 0.344*(uyv.b - delta);
    fragColor.b = uyv.g + 1.773*(uyv.b - delta);
    fragColor.a = 1.0;
    */
}
