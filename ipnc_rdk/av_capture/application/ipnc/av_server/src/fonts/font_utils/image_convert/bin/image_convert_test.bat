image_convert -i input.yuv uyvy422 640 480 640 -o output_01_565.rgb rgb565  640
image_convert -i input.yuv uyvy422 640 480 640 -o output_02_422.yuv yyuv422 640
image_convert -i input.yuv uyvy422 640 480 640 -o output_03_420.yuv yyuv420 640
image_convert -i input.yuv uyvy422 640 480 640 -o output_04_422.yuv yuv422p 640
image_convert -i input.yuv uyvy422 640 480 640 -o output_05_420.yuv yuv420p 640
image_convert -i input.yuv uyvy422 640 480 640 -o output_06_444.yuv yuv444i 640
image_convert -i input.yuv uyvy422 640 480 640 -o output_07_444.yuv yuv444p 640
image_convert -i input.yuv uyvy422 640 480 640 -o output_08_422.yuv yyvu422 640
image_convert -i input.yuv uyvy422 640 480 640 -o output_09_420.yuv yyvu420 640
image_convert -i input.yuv uyvy422 640 480 640 -o output_0A_888.rgb rgb888i 640
image_convert -i input.yuv uyvy422 640 480 640 -o output_0B_888.rgb rgb888p 640

image_convert -i output_01_565.rgb rgb565  640 480 640 -o output_11_422i.yuv uyvy422 640
image_convert -i output_02_422.yuv yyuv422 640 480 640 -o output_12_422i.yuv uyvy422 640
image_convert -i output_03_420.yuv yyuv420 640 480 640 -o output_13_422i.yuv uyvy422 640
image_convert -i output_04_422.yuv yuv422p 640 480 640 -o output_14_422i.yuv uyvy422 640
image_convert -i output_05_420.yuv yuv420p 640 480 640 -o output_15_422i.yuv uyvy422 640
image_convert -i output_06_444.yuv yuv444i 640 480 640 -o output_16_422i.yuv uyvy422 640
image_convert -i output_07_444.yuv yuv444p 640 480 640 -o output_17_422i.yuv uyvy422 640
image_convert -i output_08_422.yuv yyvu422 640 480 640 -o output_18_422i.yuv uyvy422 640
image_convert -i output_09_420.yuv yyvu420 640 480 640 -o output_19_422i.yuv uyvy422 640
image_convert -i output_0A_888.rgb rgb888i 640 480 640 -o output_19_422i.yuv uyvy422 640
image_convert -i output_0B_888.rgb rgb888p 640 480 640 -o output_19_422i.yuv uyvy422 640







