using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public struct Vector2
{
    public float x;
    public float y;

    public Vector2(float x, float y)
    {
        this.x = x;
        this.y = y;
    }
}

public struct Vector3
{     public float x;
    public float y;
    public float z;

    public Vector3(float x, float y, float z)
    {
        this.x = x;
        this.y = y;
        this.z = z;
    }
}

public struct Vector4
{
    public float x;
    public float y;
    public float z;
    public float w;

    public Vector4(float x, float y, float z, float w)
    {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }
}

namespace AOTDefs
{
    public struct NotHFA
    {
        public float x;
        public float y;
        public int z;
    }

    public struct EmbedHFA
    {
        public Vector2 x;
        public float y;
    }

    public struct EmbedHFA2
    {
        public Vector2 x;
        public Vector2 y;
    }
}
