#include <stdio.h>
#include <stdlib.h>
#include "gbuffer.h"
#include "raylibext.h"

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

float random()
{
    return (float)rand()/(float)RAND_MAX;
}

unsigned int generate_noise()
{
    
    float ssaoNoise[16*3];
    for (int i = 0; i < 16*3; i+=3) {
        ssaoNoise[i+0] = (float)(rand()/(float)RAND_MAX)*2.0-1.0;
        ssaoNoise[i+1] = (float)(rand()/(float)RAND_MAX)*2.0-1.0;
        ssaoNoise[i+2] = 0.f;
    }
    
    unsigned int noise;
    glGenTextures(1, &noise);
    glBindTexture(GL_TEXTURE_2D, noise);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return noise;
}

int main(int argc, char** argv)
{
    
    InitWindow(1280, 720, "gbuffer");
    SetTargetFPS(60);
    SetExitKey(KEY_F12);
    
    
    Camera camera = {{0, 3.5, 3}, {0, 3, 0}, {0, 1, 0}, 90.f, CAMERA_PERSPECTIVE};
    SetCameraMode(camera, CAMERA_FIRST_PERSON);
    
    SetMousePosition(Vector2Zero());
    // load shaders
    Shader gbuffer_shader = LoadShader("assets/shaders/gbuffer.vs", "assets/shaders/gbuffer.fs");
    gbuffer_shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(gbuffer_shader, "modelMatrix");
    
    Shader ssao = LoadShader(0, "assets/shaders/ssao.fs");
    BindShaderTexture(ssao, "normalbuffer", 1);
    BindShaderTexture(ssao,  "depthbuffer", 2);
    BindShaderTexture(ssao,  "noisebuffer", 3);
    
    Shader blur = LoadShader(0, "assets/shaders/blur.fs");
    BindShaderTexture(blur, "ssaobuffer", 1);
    
    Shader lighting = LoadShader(0, "assets/shaders/lighting.fs");
    BindShaderTexture(lighting,    "colorbuffer", 1);
    BindShaderTexture(lighting,   "normalbuffer", 2);
    BindShaderTexture(lighting, "positionbuffer", 3);
    BindShaderTexture(lighting,     "ssaobuffer", 4);
    
    // load models
    Model model = LoadModel("assets/models/Arcade2.obj");
    SetModelShader(&model, gbuffer_shader);
    SetModelMap(&model, MAP_DIFFUSE, LoadTexture("assets/textures/Arcade2_texture.png"));
    SetModelMap(&model, MAP_METALNESS, GetTextureDefault());
    SetModelMap(&model, MAP_NORMAL, GetTextureDefault());
    
    Model screen = LoadModel("assets/models/Arcade2_screen.obj");
    SetModelShader(&screen, gbuffer_shader);
    SetModelMap(&screen, MAP_DIFFUSE, LoadTexture("assets/textures/crt.png"));
    SetModelMap(&screen, MAP_SPECULAR, GetTextureDefault());
    SetModelMap(&screen, MAP_NORMAL, GetTextureDefault());
    
    Model teapot = LoadModel("assets/models/utah-teapot.obj");
    SetModelShader(&teapot, gbuffer_shader);
    SetModelMap(&teapot, MAP_DIFFUSE, GetTextureDefault());
    SetModelMap(&teapot, MAP_SPECULAR, GetTextureDefault());
    SetModelMap(&teapot, MAP_NORMAL, GetTextureDefault());
    
    Model bunny = LoadModel("assets/models/bunny.obj");
    SetModelShader(&bunny, gbuffer_shader);
    SetModelMap(&bunny, MAP_DIFFUSE, GetTextureDefault());
    SetModelMap(&bunny, MAP_SPECULAR, GetTextureDefault());
    SetModelMap(&bunny, MAP_NORMAL, GetTextureDefault());
    
    Model ground = LoadModelFromMesh(GenMeshPlane(100, 100, 5, 5));
    SetModelShader(&ground, gbuffer_shader);
    SetModelMap(&ground, MAP_DIFFUSE, GetTextureDefault());
    SetModelMap(&ground, MAP_SPECULAR, GetTextureDefault());
    SetModelMap(&ground, MAP_NORMAL, GetTextureDefault());
    
    Mesh light_mesh = GenMeshSphere(1, 10, 10);
    Model light = LoadModelFromMesh(light_mesh);
    SetModelShader(&light, gbuffer_shader);
    SetModelMap(&light, MAP_DIFFUSE, GetTextureDefault());
    SetModelMap(&light, MAP_SPECULAR, GetTextureDefault());
    SetModelMap(&light, MAP_NORMAL, GetTextureDefault());
    
    // set light uniforms
    SetShaderVector3(lighting, "lights[0].position", (Vector3){0, 3, 1.5});
    SetShaderVector3(lighting, "lights[0].color",    (Vector3){0, 1, 1});
    SetShaderFloat(lighting, "lights[0].linear",   0.7);
    SetShaderFloat(lighting, "lights[0].quadratic",1.8);
    
    SetShaderVector3(lighting, "lights[1].position", (Vector3){1, 3, 0});
    SetShaderVector3(lighting, "lights[1].color",    (Vector3){0, 1, 0});
    SetShaderFloat(lighting, "lights[1].linear",   0.7);
    SetShaderFloat(lighting, "lights[1].quadratic",1.8);
    
    SetShaderVector3(lighting, "lights[2].position", (Vector3){-1, 3, 0});
    SetShaderVector3(lighting, "lights[2].color",    (Vector3){1, 0, 0});
    SetShaderFloat(lighting, "lights[2].linear",   0.7);
    SetShaderFloat(lighting, "lights[2].quadratic",1.8);
    
    SetShaderVector3(lighting, "lights[3].position", (Vector3){0, 0.5, 0});
    SetShaderVector3(lighting, "lights[3].color",    (Vector3){1, 1, 1});
    SetShaderFloat(lighting, "lights[3].linear",   0.7);
    SetShaderFloat(lighting, "lights[3].quadratic",1.8);
    
    // ssao samples and noise
    for (int i = 0; i < 64; i++) {
        Vector3 sample = {((float)rand()/(float)RAND_MAX) * 2.0 - 1.0, ((float)rand()/(float)RAND_MAX) * 2.0 - 1.0, ((float)rand()/(float)RAND_MAX)};
        sample = Vector3Normalize(sample);
        sample = Vector3Multiply(sample, (float)rand()/(float)RAND_MAX);
        float scale = (float)i/64.f;
        
        scale = lerp(0.1f, 1.f, scale*scale);
        sample = Vector3Multiply(sample, scale);
        
        SetShaderVector3(ssao, FormatText("samples[%d]", i), sample);
    }
    unsigned int noise = generate_noise();
    
    // framebuffers
    gbuffer_t gbuffer = gbuffer_new(1280, 720);
    
    RenderTexture2D t = LoadRenderTexture(1280, 720);
    RenderTexture2D ssao_buffer = LoadRenderTexture(1280, 720);
    RenderTexture2D ssao_blurred = LoadRenderTexture(1280, 720);
    
    // debug
    bool ssao_enabled = true;
    bool gbuffer_enabled = true;
    
    while (!WindowShouldClose())
    {
        UpdateCamera(&camera);
        SetShaderVector3(lighting, "viewpos", camera.position);
        
        if (IsKeyPressed(KEY_F1)) ssao_enabled = !ssao_enabled;
        if (IsKeyPressed(KEY_F2)) gbuffer_enabled = !gbuffer_enabled;
        
        BeginDrawing();
            ClearBackground(BLACK);
            
            // SetShaderVector3(lighting, "lights[0].position", (Vector3){0, 3+cos(GetTime()), 1.5});
            // SetShaderVector3(lighting, "lights[1].position", (Vector3){1, 3, cos(GetTime())});
            // SetShaderVector3(lighting, "lights[2].position", (Vector3){-1, 3, -cos(GetTime())});
            // SetShaderVector3(lighting, "lights[3].position", (Vector3){-sin(GetTime())*2, 0.5, -cos(GetTime())*2});
            
            gbuffer_begin(&gbuffer);
                BeginMode3D(camera);;
                    DrawModel(model, Vector3Zero(), 1.f, WHITE);
                    DrawModel(screen, Vector3Zero(), 1.f, WHITE);
                    DrawModel(bunny, (Vector3){0, 2.75, 1}, 1.f, WHITE);
                    // DrawModel(light, (Vector3){0, 3+cos(GetTime()), 1.5}, 0.1, BLUE);
                    // DrawModel(light, (Vector3){1, 3, cos(GetTime())}, 0.1, BLUE);
                    // DrawModel(light, (Vector3){-1, 3, -cos(GetTime())}, 0.1, BLUE);
                    // DrawModel(light, (Vector3){-sin(GetTime())*2, 0.5, -cos(GetTime())*2}, 0.1, BLUE);
                    DrawModel(ground, Vector3Zero(), 1.f, WHITE);
                EndMode3D();
            gbuffer_end();
            
            BeginTextureMode(ssao_buffer); BeginShaderMode(ssao);
                SetShaderTexture(gbuffer.normal, 1);
                SetShaderTexture(gbuffer.depth, 2);
                SetShaderTexturei(noise, 3);
                DrawTextureFlipped(t.texture);
            EndShaderMode(); EndTextureMode();
            
            BeginTextureMode(ssao_blurred); BeginShaderMode(blur);
                SetShaderTexture(ssao_buffer.texture, 1);
                DrawTextureFlipped(t.texture);
            EndShaderMode(); EndTextureMode();
            
            if (gbuffer_enabled) {
                BeginShaderMode(lighting);
                    SetShaderTexture(gbuffer.color, 1);
                    SetShaderTexture(gbuffer.normal, 2);
                    SetShaderTexture(gbuffer.position, 3);
                    if (ssao_enabled) SetShaderTexture(ssao_blurred.texture, 4);
                    else SetShaderTexture(GetTextureDefault(), 4);
                    DrawTextureFlipped(t.texture);
                EndShaderMode();
            }
            else {
                DrawTextureFlipped(gbuffer.color);
            }
            
            DrawText("Deferred rendering in raylib.", 10, 10, 20, RED);
            DrawText(FormatText("F1 = Toggle SSAO = %s", ssao_enabled ? "ON" : "OFF"), 10, 35, 20, GREEN);
            DrawText(FormatText("F2 = Toggle GBuffers = %s", gbuffer_enabled ? "ON" : "OFF"), 10, 60, 20, GREEN);
        
        EndDrawing();
    }
    
    UnloadModel(model);
    UnloadModel(screen);
    UnloadModel(teapot);
    UnloadModel(bunny);
    UnloadModel(ground);
    UnloadModel(light);
    UnloadShader(gbuffer_shader);
    UnloadShader(ssao);
    UnloadShader(blur);
    UnloadShader(lighting);
    UnloadRenderTexture(t);
    UnloadRenderTexture(ssao_buffer);
    UnloadRenderTexture(ssao_blurred);
    gbuffer_free(&gbuffer);
    CloseWindow();
    
    return 0;
}