#include <cppgl.h>
#include <GL/glew.h>
#include <GL/gl.h>


using namespace cppgl;

class Lightfield {
public:
    int imageWidth = 0;    //Width of the interlaced image of the lightfield display in pixels
    int imageHeight = 0;   //Height of the interlaced image of the lightfield display in pixels

    Lightfield();
    inline virtual ~Lightfield();
    void setLightfieldParameters();
    void calculateRotatedBoundingBoxDimensions();
    void getFrustumParameters();
    void setupQuilts();
    void viewRendering(bool ourAlgorithm);
    void interlacing(bool ourAlgorithm);

private:
    //Display specific parameters
    float pitch = 1.0f;         //Number of repetitions of all views in the interlaced image on the x-axis
    float tilt = 0.0f;          //Normalized x-axis offset from top to bottom of a rendered strip resulting from the strip rotation
    float center = 0.0f;	    //Normalized constant view shift of all strips
    float subp = 0.0f;          //Subpixel view offset in normalized texture coordinates
    bool invert = 0;            //Determine whether to invert the interlacing pattern
    float viewCone = 1.0f;      //Rendered angle range in degrees
    float tiltAngle = 0.5f;     //Tilt angle of the individual strips in the interlaced image in radians
    float aspectRatio = 0.5f;   //Lightfield display aspect ratio = aspect ratio of interlaced image    

    //User specific parameters
    float fov = 0.0f;           //User defined field of view parameter in radians
    int number_of_views = 1;    //Number of rendered views, viewNumber = rows*columns
    int rows = 1;               //Number of rows in the quilt holding the rendered views
    int columns = 1;            //Number of columns in the quilt holding the rendered views
    int oldquiltwidth = 1;      //Original quilt width in pixels
    int oldquiltheight = 1;     //Original quilt height in pixels

    //Parameters resulting from our new rendering algorithm
    int newquiltwidth = 1;      //Quilt width resulting from our new algorithm in pixels
    int newquiltheight = 1;     //Quilt height resulting from our new algorithm in pixels
    float fovFactor = 1.0f;     //Factor by which to scale the original fild of view for the adapted projective mapping
    float aspectRatioNew = 0.5f;//Adapted aspect ratio of the rotated bounding box of the interlaced image
    glm::ivec2 BBDimensionsView = glm::ivec2(1,1);    //Dimensions of the rotated bounding box of a single view in pixels
    glm::ivec2 BBDimensionsImage = glm::ivec2(1,1);   //Dimensions of the rotated bounding box of the interlaced image in pixels
    float partial_repeats_outside = 0.0f;           //Partial repetitions of all views outside the rotated bounding box that have to be enclosed by adapting the new aspect ratio
    int partial_repeat_tl = 0;  //Partial repetition of all views outside of the top left corner of the rotated bounding box of the interlaced image
    int diagonal_pitch = 1;     //Full pitch in the image diagonal resulting from the rotation with the tilt angle


    glm::ivec2 getRotatedBBDimensions(float lenx, float leny);
    float getIndex(float x, float y, float pitch);
    std::vector<glm::mat4> generateFrustaMatrices(glm::mat4 currentViewMatrix, int i, bool ourAlgorithm);

};



Lightfield::Lightfield() {
}



Lightfield::~Lightfield() {
}



//Enter light field parameters of your lightfield display and choose user specific parameters
//For the Looking Glass these can be found under /LKG_calibration/visual.json
void Lightfield::setLightfieldParameters() {
    //Display specific parameters
    pitch = 246.88290405273438f;
    tilt = -0.18651899695396423f;
    center = 0.293739200f;
    subp = 0.000217013891f;
    imageWidth = 1536;
    imageHeight = 2048;
    invert = 1;
    viewCone = 40.0f;

    //User specific parameters  
    fov = glm::radians(14.0f);
    number_of_views = 48;
    rows = 6;
    columns = 8;
    oldquiltwidth = 3360;
    oldquiltheight = 3360;

    //Derived parameters
    aspectRatio = float(imageWidth) / float(imageHeight);
    tiltAngle = atan((tilt * imageWidth) / imageHeight);
}




//Calculates the rotated bounding box dimensions of individual views and the interlaced image
void Lightfield::calculateRotatedBoundingBoxDimensions() {
    //Get rotated bounding box of individual views
    float viewWidth = roundf(float(oldquiltwidth) / columns);
    float viewHeight = roundf(float(oldquiltheight) / rows);
    BBDimensionsView = getRotatedBBDimensions(viewWidth, viewHeight);

    //Get rotated bounding box of original interlaced image
    BBDimensionsImage = getRotatedBBDimensions(float(imageWidth), float(imageHeight));
}




//Helper function to calculate the dimensions of the rotated bounding box of (w,h)
glm::ivec2 Lightfield::getRotatedBBDimensions(float w, float h) {
    std::vector<glm::vec2> edges; std::vector<glm::vec2> transformedEdges;
    glm::vec2 topleft = glm::vec2(-w / 2, h / 2); edges.push_back(topleft);
    glm::vec2 topright = glm::vec2(w / 2, h / 2); edges.push_back(topright);
    glm::vec2 bottomleft = glm::vec2(-w / 2, -h / 2); edges.push_back(bottomleft);
    glm::vec2 bottomright = glm::vec2(w / 2, -h / 2); edges.push_back(bottomright);

    //rotate points with tilt angle
    for (int i = 0; i < 4; i++) {
        transformedEdges.push_back(glm::vec2(edges.at(i).x * cos(tiltAngle) + edges.at(i).y * sin(tiltAngle),
            -edges.at(i).x * sin(tiltAngle) + edges.at(i).y * cos(tiltAngle)));
    }
    float minx = 10000; float maxx = -10000; float miny = 10000; float maxy = -10000;
    for (int i = 0; i < 4; i++) {
        if (transformedEdges.at(i).x < minx) minx = transformedEdges.at(i).x;
        if (transformedEdges.at(i).x > maxx) maxx = transformedEdges.at(i).x;
        if (transformedEdges.at(i).y < miny) miny = transformedEdges.at(i).y;
        if (transformedEdges.at(i).y > maxy) maxy = transformedEdges.at(i).y;
    }
    int w_b = int(glm::abs(minx) + glm::abs(maxx));
    int h_b = int(glm::abs(miny) + glm::abs(maxy));

    return glm::ivec2(w_b, h_b);
}




//Gets view index for given pitch and texture coordinates (x,y)
float Lightfield::getIndex(float x, float y, float pitch) {
    //Get index for green subpixel as shown in shader
    float index = (x + y * tilt - subp) * pitch - center; 
    index = glm::mod(index + glm::ceil(glm::abs(index)), 1.0f);
    if (invert) index *= -1;
    index *= number_of_views;
    index = floor(index);
    return index;
}




//Gets parameters for the new quilt and projection matrix
void Lightfield::getFrustumParameters() {
        //Calculate view indices at the corners of the interlaced image
        float i_tl = getIndex(0.0f, 1.0f, pitch);
        float i_br = getIndex(1.0f, 0.0f, pitch);
        float i_bl = getIndex(0.0f, 0.0f, pitch);
        float i_tr = getIndex(1.0f, 1.0f, pitch);
        float i_br_f = getIndex(1.0f, 0.0f, floor(pitch));
        float i_tr_f = getIndex(1.0f, 1.0f, floor(pitch));
        //Depending on the tilt angle the required indices for the calculations below change
        if (tiltAngle > 0) {
            i_tl = i_br;
            i_bl = i_tl;
            i_br = i_tr;
            i_br_f = i_tr_f;}

        //Calculate the pitch in the rotated bounding box of the interlaced image along the diagonal
        //Get the width of the rotated bounding box outside the original render area
        float w_outside = glm::tan(abs(tiltAngle)) * imageHeight;
        //Get the width of an individual full repetition of views
        float repeat_w = imageWidth / pitch;
        diagonal_pitch = int(ceil(pitch) + 1 + floor(w_outside / repeat_w + (i_bl + 1) / number_of_views));
 
        //Calculate how much the aspect ratio has to be adapted to enclose a full number of view repetitions
        partial_repeats_outside = (number_of_views - (i_tl + 1)) / number_of_views + (i_br + 1) / number_of_views;
        partial_repeat_tl = int(glm::mod(round((partial_repeats_outside) * number_of_views) + i_br_f + round((pitch - floor(pitch)) * number_of_views), float(number_of_views)));

        //Calculate new aspect ratio and field of view scaling factor
        aspectRatioNew = (BBDimensionsImage.x + partial_repeats_outside * (repeat_w / (cos(abs(tiltAngle))))) / BBDimensionsImage.y;
        fovFactor = float(BBDimensionsImage.y) / imageHeight;

        //Get new quilt dimensions 
        newquiltwidth = diagonal_pitch * columns;
        newquiltheight = BBDimensionsView.y * rows;   
}




//Intializes the quilts for standard rendering and our adapted algorithm
void Lightfield::setupQuilts() {
    //New quilt resulting from our algorithm
    Framebuffer newquilt = Framebuffer("newquilt", newquiltwidth, newquiltheight);
    newquilt->attach_depthbuffer(Texture2D("newquilt_depth", newquiltwidth, newquiltheight, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT));
    newquilt->attach_colorbuffer(Texture2D("newquilt_col", newquiltwidth, newquiltheight, GL_RGBA32F, GL_RGBA, GL_FLOAT));
    newquilt->check();

    //Original quilt
    Framebuffer oldquilt = Framebuffer("oldquilt", oldquiltwidth, oldquiltheight);
    oldquilt->attach_depthbuffer(Texture2D("oldquilt_depth", oldquiltwidth, oldquiltheight, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT));
    oldquilt->attach_colorbuffer(Texture2D("oldquilt_col", oldquiltwidth, oldquiltheight, GL_RGBA32F, GL_RGBA, GL_FLOAT));
    oldquilt->check();
}




//Renders the scene #number_of_views times to the quilt using view dependent view and projection matrices
void Lightfield::viewRendering(bool ourAlgorithm) {
    //save the viewports for the total quilts
    GLint viewportO[4] = { 0,0, oldquiltwidth, oldquiltheight };
    GLint viewportN[4] = { 0,0, newquiltwidth, newquiltheight };
    int qs_viewWidth; int qs_viewHeight;

    if (ourAlgorithm) {
        glGetIntegerv(GL_VIEWPORT, viewportN);
        //render all drawelements into the new quilt
        Framebuffer::find("newquilt")->bind();
        //get quilt view dimensions
        qs_viewWidth = int(round(float(newquiltwidth) / columns));
        qs_viewHeight = int(round(float(newquiltheight) /rows));
    }
    else {
        glGetIntegerv(GL_VIEWPORT, viewportO);
        //render all drawelements into the original quilt
        Framebuffer::find("oldquilt")->bind();
        //get quilt view dimensions
        qs_viewWidth = int(round(float(oldquiltwidth) / columns));
        qs_viewHeight = int(round(float(oldquiltwidth) / rows));
    }   

    //render all views and copy each view to the quilt
    for (int viewIndex = 0; viewIndex < number_of_views; viewIndex++) {
        //get the x and y origin for this view
        int x = (viewIndex % int(columns)) * (qs_viewWidth);
        int y = int(float(viewIndex) / columns) * (qs_viewHeight);

        //set the viewport to the view to control the projection extent
        glViewport(x, y, qs_viewWidth, qs_viewHeight);

        //set the scissor to the view to restrict calls like glClear from making modifications
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, qs_viewWidth, qs_viewHeight);

        //get different view and projection matrices for every view index
        std::vector<glm::mat4> lightfieldMatrices = generateFrustaMatrices(Camera::find("std")->view, viewIndex, ourAlgorithm);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //render the teapot using the aquired matrices
        for (const auto& [key, drawelement] : Drawelement::map) {
            drawelement->bind();
            drawelement->shader->uniform("view", lightfieldMatrices.at(0));
            drawelement->shader->uniform("proj", lightfieldMatrices.at(1));
            drawelement->draw();
            drawelement->unbind();
        }
        //reset viewport
        if (ourAlgorithm) {
            glViewport(viewportN[0], viewportN[1], viewportN[2], viewportN[3]);
            //restore scissor
            glDisable(GL_SCISSOR_TEST);
            glScissor(viewportN[0], viewportN[1], viewportN[2], viewportN[3]);
        }
        else {
            glViewport(viewportO[0], viewportO[1], viewportO[2], viewportO[3]);
            //restore scissor
            glDisable(GL_SCISSOR_TEST);
            glScissor(viewportO[0], viewportO[1], viewportO[2], viewportO[3]);
        }      
    }
    ourAlgorithm? Framebuffer::find("newquilt")->unbind() : Framebuffer::find("oldquilt")->unbind();
}





//Calculates the view dependant view and projection matrices
std::vector<glm::mat4> Lightfield::generateFrustaMatrices(glm::mat4 currentViewMatrix, int viewIndex, bool ourAlgorithm) {

    //Adapted Projective Mapping
    if (ourAlgorithm) {
        //Calculate and use the original view matrix
        float cameraDistance = -1.0f / tan(fov / 2.0f);
        float offsetAngle = (float(viewIndex) / (float(number_of_views) - 1.0f) - 0.5f) * glm::radians(viewCone);
        float offset = cameraDistance * tan(offsetAngle); // calculate the offset that the camera should move
        glm::vec3 offsetLocal = glm::vec3(currentViewMatrix * glm::vec4(offset, 0.0f, cameraDistance, 1.0f));
        glm::mat4 viewMatrix = glm::translate(currentViewMatrix, offsetLocal);


        //Calculate the new adapted projection matrix
        //init P with an aspect ratio of 1 and the user defined field of view
        glm::mat4 projectionMatrix = glm::perspective(double(fov), 1.0, 0.1, 100.0);

        //modify the projection matrix, relative to the camera size and aspect ratio (=1) like before
        projectionMatrix[2][0] += offset / 1.0f;

        //apply the field of view scaling factor 
        projectionMatrix[0][0] /= fovFactor;
        projectionMatrix[1][1] /= fovFactor;
        projectionMatrix[2][0] /= fovFactor;

        //init a rotation matrix R with aspectRatio 1
        glm::mat4 rotateM = glm::rotate(glm::mat4(1), float(tiltAngle), glm::vec3(0, 0, -1));
        //adjust R to new aspect ratio and add to P
        rotateM[0][0] /= aspectRatioNew;
        rotateM[1][0] /= aspectRatioNew;
        projectionMatrix = rotateM * projectionMatrix;

        //Calculate the adapted skew parameter si'
        //width of a pixel strip in view space
        float width = 2.0f / (number_of_views * diagonal_pitch);
        //view index dependent offset si
        float si = (number_of_views - 1 - viewIndex) * width;
        //offset s' to align frustum with the correct index in the top left corner of the interlaced image
        float s = (round((partial_repeats_outside / 2) * number_of_views) - partial_repeat_tl) * width;
        //apply new offset si'
        projectionMatrix[2][0] += si + s;


        //Return view and projection matrix
        std::vector<glm::mat4> result;
        result.push_back(viewMatrix);
        result.push_back(projectionMatrix);
        return result;
    }

    //Standard Projective Mapping
    else {
        //Calculate the lightfield view matrix
        float cameraDistance = -1.0f / tan(fov / 2.0f);
        float offsetAngle = (float(viewIndex) / (float(number_of_views) - 1.0f) - 0.5f) * glm::radians(viewCone);
        float offset = cameraDistance * tan(offsetAngle);
        glm::vec3 offsetLocal = glm::vec3(currentViewMatrix * glm::vec4(offset, 0.0f, cameraDistance, 1.0f));
        glm::mat4 viewMatrix = glm::translate(currentViewMatrix, offsetLocal);

        //Calculate the lightfield projection matrix
        glm::mat4 projectionMatrix = glm::perspective(double(fov), double(aspectRatio), 0.1, 100.0);
        projectionMatrix[2][0] += offset / aspectRatio;

        //Return view and projection matrix
        std::vector<glm::mat4> result;
        result.push_back(viewMatrix);
        result.push_back(projectionMatrix);
        return result;
    }

}





//Passes the necessary parameters to the interlacing shader and constructs the interlaced image
void Lightfield::interlacing(bool ourAlgorithm) {    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        static Shader our_efficient_interlacing_shader = Shader("our_efficient_interlacing_shader", "interlacingShader.vs", "interlacingShaderEfficient.fs");
        static Shader standard_interlacing_shader = Shader("standard_interlacing_shader", "interlacingShader.vs", "interlacingShaderStandard.fs");

        //Use the adapted efficient interlacing shader for our algorithm
        if (ourAlgorithm) {
            our_efficient_interlacing_shader->bind();
            our_efficient_interlacing_shader->uniform("pitch", pitch);
            our_efficient_interlacing_shader->uniform("tilt", tilt);        
            our_efficient_interlacing_shader->uniform("center", center);
            our_efficient_interlacing_shader->uniform("invView", invert);
            our_efficient_interlacing_shader->uniform("subp", subp);
            our_efficient_interlacing_shader->uniform("tile", glm::vec3(columns, rows, number_of_views));
            our_efficient_interlacing_shader->uniform("quilt", Framebuffer::find("newquilt")->color_textures[0], 0);

            our_efficient_interlacing_shader->uniform("tilt_angle", tiltAngle);
            our_efficient_interlacing_shader->uniform("pitch_d", float(diagonal_pitch));
            our_efficient_interlacing_shader->uniform("aspect_ratio", aspectRatio);
            our_efficient_interlacing_shader->uniform("fovFactor", fovFactor);
            our_efficient_interlacing_shader->uniform("pr_tl", partial_repeat_tl);
            our_efficient_interlacing_shader->uniform("pr_total", int(partial_repeats_outside * number_of_views));
            our_efficient_interlacing_shader->uniform("aspect_ratio_n", aspectRatioNew);

            Quad::draw();

            our_efficient_interlacing_shader->unbind();
        }

        //Use the standard interlacing shader for standard rendering
        else {
            standard_interlacing_shader->bind();
            standard_interlacing_shader->uniform("pitch", pitch);
            standard_interlacing_shader->uniform("tilt", tilt); 
            standard_interlacing_shader->uniform("center", center);
            standard_interlacing_shader->uniform("invView", invert);
            standard_interlacing_shader->uniform("subp", subp);
            standard_interlacing_shader->uniform("tile", glm::vec3(columns, rows, number_of_views));
            standard_interlacing_shader->uniform("quilt", Framebuffer::find("oldquilt")->color_textures[0], 0);

            Quad::draw();

            standard_interlacing_shader->unbind();
        }
        glFinish();
}


