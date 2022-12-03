#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

namespace Geometry
{
    // Axis alligned bounding box
    struct AABB
    {
        glm::vec3 mMin;
        glm::vec3 mMax;

        AABB();
        AABB(const float& pLowX, const float& pHighX, const float& pLowY, const float& pHighY, const float& pLowZ, const float& pHighZ);
        AABB(const glm::vec3& pLowPoint, const glm::vec3& pHighPoint);

        glm::vec3 getSize() const;
        glm::vec3 getCenter() const;
        bool contains(const AABB& pAABB) const;
        bool operator== (const AABB& pOther) const;
        bool operator != (const AABB& pOther) const { return !(*this == pOther); }

        // Return a bounding box encompassing both bounding boxes.
        static AABB unite(const AABB& pAABB, const AABB& pAABB2);
        static AABB unite(const AABB& pAABB, const glm::vec3& pPoint);
        // Returns an encompassing AABB after translating and transforming pAABB.
        static AABB transform(const AABB& pAABB, const glm::vec3& pTranslate, const glm::mat4& pTransform);
    };
}// namespace Geometry