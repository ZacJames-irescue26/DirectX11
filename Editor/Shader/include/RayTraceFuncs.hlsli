struct Ray
{
    float3 origin;
    float3 dir;
};


bool RayTriangleIntersect(
    Ray ray,
    float3 p0,
    float3 p1,
    float3 p2,
    out float t,
    out float u,
    out float v)
{
    const float EPSILON = 1e-6;

    float3 edge1 = p1 - p0;
    float3 edge2 = p2 - p0;

    float3 pvec = cross(ray.dir, edge2);
    float det = dot(edge1, pvec);
    if (abs(det) < EPSILON)
        return false;
    float invDet = 1.0 / det;

    float3 tvec = ray.origin - p0;
    u = dot(tvec, pvec) * invDet;
    if (u < 0 || u > 1)
        return false;

    float3 qvec = cross(tvec, edge1);
    v = dot(ray.dir, qvec) * invDet;
    if (v < 0 || u + v > 1)
        return false;

    t = dot(edge2, qvec) * invDet;
    if (t < EPSILON)
        return false;

    return true;
}

// Returns true if ray (orig + t*dir) hits the AABB [bmin,bmax] within t in [tMin,tMax].
bool RayIntersectsAABB(
    float3 orig,
    float3 invDir, // = 1.0/dir
    float3 bmin,
    float3 bmax,
    float tMin,
    float tMax)
{
    // compute intersection t for each pair of slabs
    float3 t0 = (bmin - orig) * invDir;
    float3 t1 = (bmax - orig) * invDir;
    
    // find the near and far hits per axis
    float3 tNear3 = min(t0, t1);
    float3 tFar3 = max(t0, t1);
    
    // the ray enters the box at the maximum of the three near intersections
    float tEnter = max(max(tNear3.x, tNear3.y), tNear3.z);
    // it exits at the minimum of the three far intersections
    float tExit = min(min(tFar3.x, tFar3.y), tFar3.z);
    
    // there is an intersection if the enter is before the exit,
    // and the intersection interval overlaps [tMin,tMax]
    return (tExit >= max(tEnter, tMin)) && (tEnter <= tMax);
}


