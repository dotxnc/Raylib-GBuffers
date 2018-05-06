#include "gbuffer.h"


gbuffer_t gbuffer_new(int width, int height)
{
    gbuffer_t gbuffer;
    gbuffer_init(&gbuffer, width, height);
    return gbuffer;
}

void gbuffer_init(gbuffer_t* gbuffer, int width, int height)
{
    gbuffer->id = 0;
    gbuffer->width = width;
    gbuffer->height = height;
    
    gbuffer->color.id = 0;
    gbuffer->color.width = width;
    gbuffer->color.height = height;
    gbuffer->color.format = UNCOMPRESSED_R8G8B8A8;
    gbuffer->color.mipmaps = 0;
    
    gbuffer->normal.id = 0;
    gbuffer->normal.width = width;
    gbuffer->normal.height = height;
    gbuffer->normal.format = UNCOMPRESSED_R8G8B8A8;
    gbuffer->normal.mipmaps = 0;
    
    gbuffer->position.id = 0;
    gbuffer->position.width = width;
    gbuffer->position.height = height;
    gbuffer->position.format = UNCOMPRESSED_R8G8B8A8;
    gbuffer->position.mipmaps = 0;
    
    gbuffer->emission.id = 0;
    gbuffer->emission.width = width;
    gbuffer->emission.height = height;
    gbuffer->emission.format = UNCOMPRESSED_R8G8B8A8;
    gbuffer->emission.mipmaps = 0;
    
    glGenFramebuffers(1, &gbuffer->id);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->id);
    
    glGenTextures(1, &gbuffer->position.id);
    glBindTexture(GL_TEXTURE_2D, gbuffer->position.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gbuffer->position.id, 0);
    
    glGenTextures(1, &gbuffer->normal.id);
    glBindTexture(GL_TEXTURE_2D, gbuffer->normal.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gbuffer->normal.id, 0);
    
    glGenTextures(1, &gbuffer->color.id);
    glBindTexture(GL_TEXTURE_2D, gbuffer->color.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gbuffer->color.id, 0);
    
    unsigned int buffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, buffers);
    
    glGenTextures(1, &gbuffer->depth.id);
    glBindTexture(GL_TEXTURE_2D, gbuffer->depth.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gbuffer->depth.id, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        // TraceLog(LOG_WARNING, "Framebuffer object could not be created...");
        
        switch (status)
        {
            case GL_FRAMEBUFFER_UNSUPPORTED: break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: TraceLog(LOG_WARNING, "Framebuffer incomplete attachment"); break;
#if defined(GRAPHICS_API_OPENGL_ES2)
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: TraceLog(LOG_WARNING, "Framebuffer incomplete dimensions"); break;
#endif
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: TraceLog(LOG_WARNING, "Framebuffer incomplete missing attachment"); break;
        }
    }
    printf("\t\tFRAMEBUFFER STATUS : %d : %s\n", status, status==GL_FRAMEBUFFER_COMPLETE?"TRUE":"FALSE");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
}

void gbuffer_begin(gbuffer_t* gbuffer)
{
    rlglDraw();
    
    rlEnableRenderTexture(gbuffer->id);
    
    rlClearScreenBuffers();
    
    rlViewport(0, 0, gbuffer->width, gbuffer->height);
    
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    
    rlOrtho(0, gbuffer->width, gbuffer->height, 0, 0, 1);
    
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    
    glDisable(GL_BLEND);
}

void gbuffer_end()
{
    glEnable(GL_BLEND);
    rlglDraw();
    
    rlDisableRenderTexture();
    
    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    
    rlOrtho(0, GetScreenWidth(), GetScreenHeight(), 0, 0, 1);
    
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

void gbuffer_free(gbuffer_t* gbuffer)
{
    glDeleteFramebuffers(1, &gbuffer->id);
    glDeleteTextures(1, &gbuffer->color.id);
    glDeleteTextures(1, &gbuffer->normal.id);
    glDeleteTextures(1, &gbuffer->position.id);
}

