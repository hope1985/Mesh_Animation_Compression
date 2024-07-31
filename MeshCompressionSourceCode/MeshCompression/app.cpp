#include "GInclude.h"

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "nfd.h"

#include <windows.h>  // Library effective with Windows
#include <cstdlib>

#include "Commons.h"
#include "ShaderHandler.h"
#include "MotionStrength.h"
#include "TemporalCluster.h"
#include "SpatialSegmentation.h"
#include "GFT.h"
#include "objLoader.h"
#include "shadersPrograms.h"
#include "KGerror.h"

// ================================= signitures ================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int  action, int mods);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
// ================================= signitures ================================

//========================================== OpenFile =========================

string openfolderDialog() {
    //nfdchar_t* outPath = nullptr;
    //nfdresult_t result = NFD_OpenDialog(nullptr, nullptr, &outPath);

    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_PickFolder(nullptr, &outPath);

    string str_outPath ;
    if (result == NFD_OKAY) {
        std::cout << "Selected Folder:" << outPath << std::endl;
        str_outPath = string(outPath);
        //free(outPath);
    }
    else if (result == NFD_CANCEL) {
        std::cout << "User pressed cancel.\n";
    }
    else {
        std::cout << "Error: " << NFD_GetError() << std::endl;

    }
    return str_outPath;
    
}

//=======================================================================

// Camera parameters
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f,-5.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.02f;

string ROOTDIR_MESH_FILES = "3d_objects\\horse-gallop-light";
string OBJFILE_EXTENSION = ".obj";

int current_loaded_fn = 10;
int current_loaded_vertices = 4000;
const int defaultNumTemporalCluster = 3;
const int defaultNumSpatialSeqmentation = 7;
const double defaultNumCompressionFactor = 20;
const int defaultMaxLoadedFrames = 10;
const int defaultMaxLoadVertices = 4000;
double KGerror_value = 0.0;
char temporalCluster_text[256] = ""; 
char spatialSegments_text[256] = ""; 
char comapressionFactor_text[256] = ""; 
char Max_loaded_vertices_text[256] = "";
char Max_loaded_frame_text[256] = "";
//==========================================
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//==========================================
// mouse handling
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool mousebutton_pressed = false;
bool mousebutton_released = true;
int counter = 0;

float initAngle = 0;
float currnetAngle = 30;

//========================================
std::vector< ObjFileData> currentMesh;
std::vector< ObjFileData> orgMesh ;
std::vector<int> faces;
std::vector< Vertex3f> vertices ;
int numVertices;
int bufferVertexSize ;
int numFacesIndices ;
unsigned int VBO, VAO, EBO;

//===============Used for saving meshes=====
std::vector<vector<std::vector<double>>> recMesh;
std::vector < std::vector<int>> recFace;
//==========================================
GFT *meshAnimationGFT=nullptr;
MeshAnimationInfo meshAnimationInfo;
std::vector<std::string> filenames;
KGerror kge;
//==========================================
static bool previous_show_wireFrame = false;
static bool current_show_wireFrame = false;
//============================================
static bool obj_file_loaded = false;
static int cnt = 0;
static float previous = 0;
static float interval_time = 2.f; // two seconds
static bool forward_play = true;


GFT* process_mesh_frames(MeshAnimationInfo meshAnimationInfo,int numCluster=3,int numSeqments= 7)
{
    VectorX<VectorX<Vector3d>> vertices = meshAnimationInfo.vertices;
    auto motionSt = MotionStrength(vertices);
    motionSt.computeDerivations();
    motionSt.computeCurvatures();
    motionSt.computeTorsions();

    //Compute Temporal Cluster
    TemporalCluster tc(motionSt.curvatureVec, motionSt.torsionVec, numCluster);
    tc.makeClusterData();
    tc.makeClustersStdVector();
    tc.clustring();

    //Compute Spatial Segments
    SpatialSegmentation ss(numSeqments, 0.5, vertices, tc.clusterIndices, numCluster, motionSt.curvatureVec, motionSt.torsionVec);
    ss.makeSegmentStdVector();
    ss.doSegmentation();

    //Finally compute Graph Fourier Transform for Mesh
    GFT *gft=new GFT(meshAnimationInfo.vertices, meshAnimationInfo.faces, ss.seqments,tc.numCluster,ss.numSegments);
    gft->makeSegmentEigenVetors();

    //Retuern GFT object such that we can cll getReconstrucetedMesh function with desired comparession factor ratio 
    return gft;
}
 
void draw_mesh( ShaderHandler shader)
{
    if (obj_file_loaded==false)
        return;

    shader.use();
   
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f,0.0f, 0.0f));
    model = glm::rotate(model, currnetAngle, glm::vec3(0.0f, 1.0f, 0.0f)); 
    //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view =glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

    //=======================================

    // Update the buffer with new data using glBufferSubData
    float now = (float)glfwGetTime();
    float delta = now - previous;
    previous = now;

    // for each timer do this
    interval_time += delta;
    if (interval_time > 0.05f)
    {
        if (forward_play)
        {
            if (cnt < current_loaded_fn)
            {
                if (previous_show_wireFrame == false)
                glBufferSubData(GL_ARRAY_BUFFER, 0, bufferVertexSize, currentMesh[cnt].vNormalizedPositins.data());
            else
                glBufferSubData(GL_ARRAY_BUFFER, 0, bufferVertexSize, currentMesh[cnt].out_vPosLinesNormalized.data());

                //glfwSetTime(0);
                interval_time = 0;
                cnt++;
            }
            else
            {
                cnt = current_loaded_fn - 1;
                forward_play = false;
            }
        }
        else
        {
            if (cnt >= 0)
            {
                if (previous_show_wireFrame == false)
                glBufferSubData(GL_ARRAY_BUFFER, 0, bufferVertexSize, currentMesh[cnt].vNormalizedPositins.data());
               else
                 glBufferSubData(GL_ARRAY_BUFFER, 0, bufferVertexSize, currentMesh[cnt].out_vPosLinesNormalized.data());

                //glfwSetTime(0);
                interval_time = 0;
                cnt--;
            }
            else
            {
                cnt = 0;
                forward_play = true;
            }
        }
    }
    glBindVertexArray(VAO);
    
    if (previous_show_wireFrame == false)
    {
        glDrawElements(GL_TRIANGLES, numFacesIndices, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0,numFacesIndices);
    }
    else
    {
        glDrawArrays(GL_LINES, 0, numVertices);
    }
    glBindVertexArray(0);
}

void update_current_mesh(std::vector<ObjFileData> inMesh)
{
    currentMesh = inMesh;
   
    faces = currentMesh[0].face_pos_idxs_minus_1;

    if (previous_show_wireFrame == false)
    {
        vertices = currentMesh[0].vNormalizedPositins;
    }
    else
    {
        vertices = currentMesh[0].out_vPosLinesNormalized;
    }
}

void reload_mesh()
{  
    try
    {
        obj_file_loaded = false;
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        numVertices = (int)vertices.size();
        bufferVertexSize = (int)(numVertices * sizeof(Vertex3f));
        numFacesIndices = (int)(faces.size());

        //===================================================

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, bufferVertexSize, &vertices[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numFacesIndices * sizeof(GLint), &faces[0], GL_DYNAMIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f), (void*)0);
        glEnableVertexAttribArray(0);

        obj_file_loaded = true;
    }
    catch (exception ex)
    {
        obj_file_loaded = false;
        std::cout<<"Error in reloading mesh..." << std::endl;
    }
}

void makeWindowSettings()
{
    //============================== ImGui==================================
// Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::SetNextWindowSize(ImVec2(SCR_WIDTH/3, SCR_WIDTH/3));
    // Set the position for the next window always
    ImVec2 windowPos(20, 20); // Adjust these coordinates as needed
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    
    ImGui::NewFrame();

    #ifdef NDEBUG  //Does not worl with debug mode!
        ImGui::Begin("Settings");
    #endif
    ImGui::Text("W,A,S,D,Q,E Keys: movining camera");
    ImGui::Text("R Key: reset the position of camera");
    ImGui::Spacing();
    ImGui::Spacing();
    //=============================================================
    // Button to open folder dialog
    if (ImGui::Button("Open Folder Dialog")) {
        ROOTDIR_MESH_FILES = openfolderDialog();

        if (ROOTDIR_MESH_FILES.empty() == false)
        {
            int max_fn = 0;
            int max_v = 0;
            try
            {
                max_fn = stoi(Max_loaded_frame_text);
            }
            catch (exception ex)
            {
                std::cout << "The value of max loaded frames is wrong." << std::endl;
                max_fn = defaultMaxLoadedFrames;

                
                string lf_str = to_string(defaultMaxLoadedFrames);
                memcpy(Max_loaded_frame_text, lf_str.c_str(), lf_str.size());
                Max_loaded_frame_text[lf_str.size()] = '\0';
            }
            try
            {
                max_v = stoi(Max_loaded_vertices_text);
            }
            catch (exception ex)
            {
                std::cout << "The value of max loaded vertices is wrong." << std::endl;
                max_v = defaultMaxLoadVertices;

                string lv_str = to_string(defaultMaxLoadVertices);
                memcpy(Max_loaded_vertices_text, lv_str.c_str(), lv_str.size());
                Max_loaded_vertices_text[lv_str.size()] = '\0';
            }

            try
            {
                filenames = getlistFilesWithExtension(ROOTDIR_MESH_FILES.c_str(), OBJFILE_EXTENSION.c_str());
                std::cout << "Loading mesh..." << std::endl;

                orgMesh = load_mesh_files(ROOTDIR_MESH_FILES.c_str(), filenames, max_fn, max_v);
                
                current_loaded_fn =(int) orgMesh.size();
                if (current_loaded_fn>0)
                current_loaded_vertices = (int)orgMesh[0].vPositins.size();

                update_current_mesh(orgMesh);
                reload_mesh();

                //Reset the camera position
                cameraPosition = glm::vec3(0,0,3.0f);

                if (meshAnimationGFT != nullptr)
                {
                    delete meshAnimationGFT;
                    meshAnimationGFT = nullptr;
                }
                std::cout << "Mesh was loaded (frames="<< orgMesh.size() <<", vertices=" <<orgMesh[0].vPositins.size()<<")." << std::endl;
            }
            catch (exception ex)
            {
                std::cout << "Error in loading object files." << std::endl;
            }
        }
    }
    ImGui::Spacing();
    ImGui::Spacing();
    //=============================================================
    ImGui::Text("Max loaded Frames:");
    ImGui::SetNextItemWidth(70);
    ImGui::InputText(" ", Max_loaded_frame_text, IM_ARRAYSIZE(Max_loaded_frame_text));
    ImGui::Spacing();
    //=============================================================
    ImGui::Text("Max loaded Vertices:");
    ImGui::SetNextItemWidth(70);
    ImGui::InputText("  ", Max_loaded_vertices_text, IM_ARRAYSIZE(Max_loaded_vertices_text));
    ImGui::Spacing();
    ImGui::Spacing();
    //=============================================================
    ImGui::Checkbox("Show wireframe", &current_show_wireFrame);
    if ( previous_show_wireFrame!= current_show_wireFrame)
    {
        previous_show_wireFrame = current_show_wireFrame; 
        update_current_mesh(currentMesh);
        reload_mesh();
    }
    ImGui::Spacing();
    ImGui::Spacing();
    //=============================================================
    ImGui::Text("Temporal Clusters:");
    ImGui::SetNextItemWidth(50);
    //Create textbox
    ImGui::InputText("   ", temporalCluster_text, IM_ARRAYSIZE(temporalCluster_text) );
    ImGui::Spacing();
    //=============================================================
    ImGui::Text("Spatial Segments:");
    ImGui::SetNextItemWidth(50);
    //default text of textbox
    ImGui::InputText("    ", spatialSegments_text, IM_ARRAYSIZE(spatialSegments_text) );
    ImGui::Spacing();
    //=============================================================
    ImGui::Text("Compression Factor(%%):");
    ImGui::SetNextItemWidth(75);
    ImGui::SameLine();
    ImGui::SameLine();
    ImGui::InputText("     ", comapressionFactor_text, IM_ARRAYSIZE(comapressionFactor_text));
    //=============================================================
    if (ImGui::Button("Compress Mesh"))
    {
        std::vector<std::vector<std::vector<double>>> rec_data;
        int numTc, numSS;
        double compressionratio;
        try
        {
            numTc = stoi(temporalCluster_text);
        }
        catch (exception ex)
        {
            std::cout << "The number of temporal cluster is wrong." << std::endl;
            numTc = defaultNumTemporalCluster;

            //default text of temporalCluster textbox
            string tc_str = to_string(defaultNumTemporalCluster);
            memcpy(temporalCluster_text, tc_str.c_str(), tc_str.size());
            temporalCluster_text[tc_str.size()] = '\0';

        }
        try
        {
            numSS = stoi(spatialSegments_text);
        }
        catch (exception ex)
        {
            std::cout << "The number of spatial segments is wrong." << std::endl;
            numSS = defaultNumSpatialSeqmentation;

            //default text of spatialSegments textbox spatialSegments
            string ss_str = to_string(defaultNumSpatialSeqmentation);
            memcpy(spatialSegments_text, ss_str.c_str(), ss_str.size());
            spatialSegments_text[ss_str.size()] = '\0';
        }

        try
        {
            compressionratio = stoi(comapressionFactor_text);
        }
        catch (exception ex)
        {
            std::cout << "The value of compressionratio is wrong." << std::endl;
            compressionratio = defaultNumCompressionFactor;

            //default text of spatialSegments textbox spatialSegments
            string cr_str = to_string(defaultNumCompressionFactor);
            memcpy(comapressionFactor_text, cr_str.c_str(), cr_str.size());
            comapressionFactor_text[cr_str.size()] = '\0';
        }

        try
        { 
            if (orgMesh.size() > 0)
            {

                rec_data.clear();
                if (meshAnimationGFT == nullptr)
                {
                    meshAnimationInfo = getMeshAnimationInfo(orgMesh, current_loaded_fn);
                    meshAnimationGFT = process_mesh_frames(meshAnimationInfo, numTc, numSS);
                    rec_data = meshAnimationGFT->getReconstrucedMesh(compressionratio);
                    kge.SetOriginalVertices(meshAnimationInfo.verticesStd);
                    KGerror_value= kge.ComputeKG_error(rec_data);

                    recMesh = rec_data;
                    recFace = meshAnimationInfo.faces;


                }
                else if (numTc != meshAnimationGFT->numCluster || numSS!= meshAnimationGFT->numSeqments)
                {
                    if (meshAnimationGFT != nullptr)
                    {
                        delete meshAnimationGFT;
                        meshAnimationGFT = nullptr;
                    }

                    meshAnimationInfo = getMeshAnimationInfo(orgMesh, current_loaded_fn);
                    meshAnimationGFT = process_mesh_frames(meshAnimationInfo, numTc, numSS);
                    rec_data = meshAnimationGFT->getReconstrucedMesh(compressionratio);
                    kge.SetOriginalVertices(meshAnimationInfo.verticesStd);
                    KGerror_value = kge.ComputeKG_error(rec_data);

                    recMesh = rec_data;
                    recFace = meshAnimationInfo.faces;
                }

                else 
                {
                  

                    double compressionratio = atof(comapressionFactor_text);
                    rec_data = meshAnimationGFT->getReconstrucedMesh(compressionratio);

                    recMesh = rec_data;
                    recFace = meshAnimationInfo.faces;

                    KGerror_value = kge.ComputeKG_error(rec_data);
                }

                if (rec_data.size() > 0)
                {
                    //std::cout << "rec_data.size()" << rec_data.size() << "\n";
                    std::vector< ObjFileData> rec_mesh(rec_data.size());
                    for (int f = 0; f < rec_data.size(); f++)
                    {
                        //std::cout <<f<< "  rec_data[f].size()" <<rec_data[f].size() << "\n";
                        for (int v = 0; v < int(rec_data[f].size()); v++)
                        {
                            Vertex3f vec3f;
                            vec3f.x = (float)rec_data[f][v][0];
                            vec3f.y = (float)rec_data[f][v][1];
                            vec3f.z = (float)rec_data[f][v][2];
                            rec_mesh[f].vPositins.push_back(vec3f);

                            float size = sqrt((vec3f.x * vec3f.x) + (vec3f.y * vec3f.y) + (vec3f.z * vec3f.z));
                            if (size < rec_mesh[f].min_size)
                            {
                                rec_mesh[f].min_size = size;
                            }
                            if (size > rec_mesh[f].max_size)
                            {
                                rec_mesh[f].max_size = size;
                            }

                        }
                        //Normalize Vertices position:
                        for (int i = 0; i<int(rec_mesh[f].vPositins.size()); i++)
                        {
                            Vertex3f p;
                            p.x = (rec_mesh[f].vPositins[i].x / rec_mesh[f].max_size);
                            p.y = (rec_mesh[f].vPositins[i].y / rec_mesh[f].max_size);
                            p.z = (rec_mesh[f].vPositins[i].z / rec_mesh[f].max_size);
                            rec_mesh[f].vNormalizedPositins.push_back(p);
                        }

                        rec_mesh[f].face_pos_idxs = orgMesh[f].face_pos_idxs;
                        rec_mesh[f].faces3v_pos_idx = orgMesh[f].faces3v_pos_idx;
                        rec_mesh[f].face_pos_idxs_minus_1= orgMesh[f].face_pos_idxs_minus_1;

                        for (int i = 0; i < rec_mesh[f].face_pos_idxs.size(); i++)
                        {

                            int val = (int)rec_mesh[f].face_pos_idxs[i];
                            //============================
                            //rec_mesh[f].out_vPosFaces.push_back(rec_mesh[f].vPositins[val - 1]);
                            rec_mesh[f].out_vPosLines.push_back(rec_mesh[f].vPositins[val - 1]);
                            if (i % 3 == 1)
                            {
                                rec_mesh[f].out_vPosLines.push_back(rec_mesh[f].vPositins[val - 1]);
                            }

                            rec_mesh[f].out_vPosLinesNormalized.push_back(rec_mesh[f].vNormalizedPositins[val - 1]);
                            if (i % 3 == 1)
                            {
                                rec_mesh[f].out_vPosLinesNormalized.push_back(rec_mesh[f].vNormalizedPositins[val - 1]);
                            }

                            //============================
                        }
                        update_current_mesh(rec_mesh);
                        reload_mesh();
                    }
                }
            }
            else
            {
                std::cout << "Mesh is not loaded." << std::endl;
            }
        }
        catch (exception ex)
        {
            std::cout << "Error in Compression process:"<< ex.what()<<std::endl;          
        }
    }
    ImGui::Spacing();
    //=============================================================
    ImGui::Text("KG-Error:");
    ImGui::SameLine();
    ImGui::Text(to_string(KGerror_value).c_str());
    ImGui::Spacing();
    //=============================================================
    // Button to save folder dialog
    if (ImGui::Button("Save File")) {
        ROOTDIR_MESH_FILES = openfolderDialog();
        if (recMesh.size()>0)
        {
            for (int i = 0; i < recMesh.size(); i++)
            {
                string  filepath = ROOTDIR_MESH_FILES + "\\frames-" + to_string(i) + ".obj";
                save_obj_file(filepath.c_str(), recFace, recMesh, i);
            }
        }
    }

    // Render ImGui
    ImGui::Render();
    //============================== ImGui==================================
}

int main()
{
    //Initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mesh Compression", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, keyCallback);

    // ================== Initialize GLEW (WARNINNG: Call after glfwMakeContextCurrent) =================
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        // Handle GLEW initialization error
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        glfwTerminate();
        return -1;
    }
    else
    {
        fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    }

    //===================================== Shaders ======================================== 
    // build and compile shaders  (NOTE : MUST BE AFTER INIT OF GLFW)
    ShaderHandler shader("vShader.vs", "fShader.fs");

    // ================================= Setup ImGui ======================================
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //==================== Init Windows Settings ========================== 

    string tc_str = to_string(defaultNumTemporalCluster);
    memcpy(temporalCluster_text, tc_str.c_str(), tc_str.size());
    temporalCluster_text[tc_str.size()] = '\0';

    string ss_str = to_string(defaultNumSpatialSeqmentation);
    memcpy(spatialSegments_text, ss_str.c_str(), ss_str.size());
    spatialSegments_text[ss_str.size()] = '\0';

    string cr_str = to_string(defaultNumCompressionFactor);
    memcpy(comapressionFactor_text, cr_str.c_str(), cr_str.size());
    comapressionFactor_text[cr_str.size()] = '\0';

    string lf_str = to_string(defaultMaxLoadedFrames);
    memcpy(Max_loaded_frame_text, lf_str.c_str(), lf_str.size());
    Max_loaded_frame_text[lf_str.size()] = '\0';

    string lv_str = to_string(defaultMaxLoadVertices);
    memcpy(Max_loaded_vertices_text, lv_str.c_str(), lv_str.size());
    Max_loaded_vertices_text[lv_str.size()] = '\0';
    //======================================================================= 

    glEnable(GL_DEPTH_TEST);

    glfwSetTime(0);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        makeWindowSettings();

        processInput(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        draw_mesh(shader);

        //swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    //terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

//Query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// Callback function for handling key presses
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    bool show = false;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_Q:
            show = true;
            cameraPosition.z += cameraSpeed ;
            break;
        case GLFW_KEY_E:
            cameraPosition.z -= cameraSpeed ;
            show = true;
            break;
        case GLFW_KEY_A:
            cameraPosition.x += cameraSpeed;
            show = true;
            break;
        case GLFW_KEY_D:
            cameraPosition.x -= cameraSpeed;
            show = true;
            break;
        case GLFW_KEY_W:
            cameraPosition.y -= cameraSpeed;
            show = true;
            break;
        case GLFW_KEY_S:
            cameraPosition.y += cameraSpeed;
            show = true;
            break;

        case GLFW_KEY_R:
            cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
            show = true;
            break;
        }
        if (show == true)
        {
            std::cout << "cameraPosition=(" << cameraPosition.x << "," << cameraPosition.y << "," << cameraPosition.z << ")\n";
       
        }
    }
}
// Whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (mousebutton_released)
    {
        lastX = 0;
        lastY = 0;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    if (mousebutton_pressed)
    {
        if (counter > 1)
        {
            if (xoffset > 0)
            {
                currnetAngle+=0.1f;
            }
            else
            {
                currnetAngle-= 0.1f;
            }
            counter = 0;
        }
        else
        {
            counter++;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int  action, int mods)
{
    if (action==GLFW_PRESS)
    {
        mousebutton_pressed = true;
    }
    else
    {
        mousebutton_pressed = false;
    }

    if (action == GLFW_RELEASE)
    {     
        mousebutton_released = true;
        initAngle = currnetAngle;
    }
    else
    {
        mousebutton_released = false;
    }
}

