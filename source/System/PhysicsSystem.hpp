#pragma once

#include "glm/vec3.hpp"

#include "Utility/Config.hpp"

namespace System
{
    class SceneSystem;
    class CollisionSystem;

    // A numerical integrator, PhysicsSystem take Transform and RigidBody components and applies kinematic equations.
    // The system is force based and numerically integrates
    class PhysicsSystem
    {
    public:
        PhysicsSystem(SceneSystem& pSceneSystem, CollisionSystem& pCollisionSystem);
        void integrate(const DeltaTime& pDeltaTime);

        size_t mUpdateCount;
        float mRestitution; // Coefficient of restitution applied in collision response.
        bool mApplyCollisionResponse;
    private:
        SceneSystem& mSceneSystem;
        CollisionSystem& mCollisionSystem;

        DeltaTime mTotalSimulationTime; // Total time simulated using the integrate function.
        glm::vec3 mGravity;             // The acceleration due to gravity.
    };
} // namespace System