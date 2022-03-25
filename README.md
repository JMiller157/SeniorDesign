# SeniorDesign
Code I wrote for my Senior Design course

It's pretty rough.

The idea is to find the real world coordinates of an object on a flat plane. 

ASSUMPTION: The bottom of a foreground object lies in a 2d plane reference plane. Works well for movement along the ground; not so much for something not touching the reference plane (something flying or jumping).

The program needs to have some values manually inputed before proper function will occur. Real world referance points need to be placed and their coorisponding pixel values need to found. A transform matrix is found that relates pixel values on the camer to a 2d plane of the reference points.

First, the program uses a Gaussian Mixture Model provided by OpenCV to find foreground objects (I found the current settings to work well on a bright sunny day with a basic Raspberry Pi camera). Second, A bounding box is drawn around foreground objects. Third, the pixel value of the bottom center of a bounding box is found. Fourth, the pixel value of the bounding box is transformed to the reference plane found earlier. The transformation results in real approximate real world coordinates being found. 
