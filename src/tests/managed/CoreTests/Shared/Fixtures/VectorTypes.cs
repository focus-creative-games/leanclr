using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace AOTDefs
{
    public struct Vector1f
    {
        public float x;

        public static float GetX(Vector1f v)
        {
            return v.x;
        }
    }

    public struct Vector2f
    {
        public float x;
        public float y;
    }

    public struct Vector3f
    {
        public float x;
        public float y;
        public float z;
    }

    public struct Vector4f
    {
        public float x;
        public float y;
        public float z;
        public float w;
    }

    public struct Vector1d
    {
        public double x;
    }

    public struct Vector2d
    {
        public double x;
        public double y;
    }

    public struct Vector3d
    {
        public double x;
        public double y;
        public double z;
    }

    public struct Vector4d
    {
        public double x;
        public double y;
        public double z;
        public double w;
    }
}