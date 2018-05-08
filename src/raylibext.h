#ifndef RAYLIBEXT_H
#define RAYLIBEXT_H

#include <raylib.h>
#include <raymath.h>
#include "glad.h"

int GetShaderUniform(Shader s, const char* name)
{
    int loc = -1;
    loc = glGetUniformLocation(s.id, name);
    return loc;
}

void BindShaderTexture(Shader s, const char* name, int id)
{
    int loc = GetShaderUniform(s, name);
    SetShaderValuei(s, loc, &id, 1);
}

void SetModelMap(Model* m, TexmapIndex i, Texture2D t)
{
    m->material.maps[i].texture = t;
}

void SetModelShader(Model* m, Shader s)
{
    m->material.shader = s;
}

void SetShaderTexture(Texture2D t, int i)
{
    glActiveTexture(GL_TEXTURE0+i);
    glBindTexture(GL_TEXTURE_2D, t.id);
}

void SetShaderTexturei(unsigned int id, int i)
{
    glActiveTexture(GL_TEXTURE0+i);
    glBindTexture(GL_TEXTURE_2D, id);
}

void SetShaderVector2(Shader s, const char* n, Vector2 v)
{
    SetShaderValue(s, GetShaderUniform(s,n), (float*)&v, 2);
}

void SetShaderVector3(Shader s, const char* n, Vector3 v)
{
    SetShaderValue(s, GetShaderUniform(s,n), (float*)&v, 3);
}

void SetShaderVector4(Shader s, const char* n, Vector4 v)
{
    SetShaderValue(s, GetShaderUniform(s,n), (float*)&v, 4);
}

void SetShaderFloat(Shader s, const char*n, float f)
{
    SetShaderValue(s, GetShaderUniform(s,n), &f, 1);
}

void SetShaderInt(Shader s, const char*n, int i)
{
    SetShaderValuei(s, GetShaderUniform(s,n), &i, 1);
}

void DrawTextureFlipped(Texture2D t)
{
    DrawTexturePro(t, (Rectangle){0, 0, t.width, -t.height}, (Rectangle){0, 0, t.width, t.height}, Vector2Zero(), 0.f, WHITE);
}

#endif
