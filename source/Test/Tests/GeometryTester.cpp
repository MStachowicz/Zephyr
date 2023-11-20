#include "GeometryTester.hpp"

#include "Geometry/AABB.hpp"
#include "Geometry/Cone.hpp"
#include "Geometry/Cylinder.hpp"
#include "Geometry/Frustrum.hpp"
#include "Geometry/Intersect.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/LineSegment.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Triangle.hpp"

#include "Utility/Stopwatch.hpp"
#include "Utility/Utility.hpp"

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui.h"
#include "OpenGL/DebugRenderer.hpp"

#include <array>

namespace Test
{
	void GeometryTester::runUnitTests()
	{
		runAABBTests();
		runTriangleTests();
		run_frustrum_tests();
		run_sphere_tests();
		run_point_tests();
	}
	void GeometryTester::runPerformanceTests()
	{
		constexpr size_t triangleCount = 1000000 * 2;
		std::vector<float> randomTrianglePoints = Utility::get_random(std::numeric_limits<float>::min(), std::numeric_limits<float>::max(), triangleCount * 3 * 3);
		std::vector<Geometry::Triangle> triangles;
		triangles.reserve(triangleCount);

		for (size_t i = 0; i < triangleCount; i++)
			triangles.emplace_back(Geometry::Triangle(
				glm::vec3(randomTrianglePoints[i], randomTrianglePoints[i + 1], randomTrianglePoints[i + 2]),
				glm::vec3(randomTrianglePoints[i + 3], randomTrianglePoints[i + 4], randomTrianglePoints[i + 5]),
				glm::vec3(randomTrianglePoints[i + 6], randomTrianglePoints[i + 7], randomTrianglePoints[i + 8])));

		auto triangleTest = [&triangles](const size_t& pNumberOfTests)
		{
			if (pNumberOfTests * 2 > triangles.size()) // Multiply by two to account for intersection calling in pairs.
				throw std::logic_error("Not enough triangles to perform pNumberOfTests. Bump up the triangleCount variable to at least double the size of the largest performance test.");

			for (size_t i = 0; i < pNumberOfTests * 2; i += 2)
				Geometry::intersecting(triangles[i], triangles[i + 1]);
		};

		auto triangleTest1 = [&triangleTest]() { triangleTest(1); };
		auto triangleTest10 = [&triangleTest]() { triangleTest(10); };
		auto triangleTest100 = [&triangleTest]() { triangleTest(100); };
		auto triangleTest1000 = [&triangleTest]() { triangleTest(1000); };
		auto triangleTest10000 = [&triangleTest]() { triangleTest(10000); };
		auto triangleTest100000 = [&triangleTest]() { triangleTest(100000); };
		auto triangleTest1000000 = [&triangleTest]() { triangleTest(1000000); };
		runPerformanceTest({"Triangle v Triangle 1", triangleTest1});
		runPerformanceTest({"Triangle v Triangle 10", triangleTest10});
		runPerformanceTest({"Triangle v Triangle 100", triangleTest100});
		runPerformanceTest({"Triangle v Triangle 1,000", triangleTest1000});
		runPerformanceTest({"Triangle v Triangle 10,000", triangleTest10000});
		runPerformanceTest({"Triangle v Triangle 100,000", triangleTest100000});
		runPerformanceTest({"Triangle v Triangle 1,000,000", triangleTest1000000});
	}

	void GeometryTester::runAABBTests()
	{
		{ // Default intiailise
			Geometry::AABB aabb;
			runUnitTest({aabb.get_size() == glm::vec3(0.f), "AABB initialise size at 0", "Expected size of default AABB to be 0"});
			runUnitTest({aabb.get_center () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
			runUnitTest({aabb.get_center () == glm::vec3(0.f), "AABB initialise to world origin", "Expected default AABB to start at [0, 0, 0]"});
		}
		{ // Initialise with a min and max
			// An AABB at low point [-1,-1,-1] to [1,1,1]
			auto aabb = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
			runUnitTest({aabb.get_size() == glm::vec3(2.f), "AABB initialised with min and max size at 2", "Expected size of AABB to be 2"});
			runUnitTest({aabb.get_center() == glm::vec3(0.f), "AABB initialise with min and max position", "Expected AABB to center at [0, 0, 0]"});
		}
		{ // Initialise with a min and max not at origin
			// An AABB at low point [1,1,1] to [5,5,5] size of 4 center at [3,3,3]
			auto aabb = Geometry::AABB(glm::vec3(1.f), glm::vec3(5.f));
			runUnitTest({aabb.get_size() == glm::vec3(4.f), "AABB initialised with min and max not at origin", "Expected size of AABB to be 4.f"});
			runUnitTest({aabb.get_center() == glm::vec3(3.f), "AABB initialised with min and max not at origin", "Expected AABB to center at [3, 3, 3]"});
		}

		{ // Tranform
		}
		{ // Unite

			{ // check result of the static AABB::unite is the same as member unite
			}
		}
		{ // Contains

		}

		{ // Intersections
			{// No touch in all directions
				const auto originAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));

				// Create an AABB in line with all 6 faces
				const auto leftAABB  = Geometry::AABB::transform(originAABB, glm::vec3(), glm::mat4(1.f), glm::vec3(1.f));
				const auto rightAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
				const auto aboveAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
				const auto belowAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
				const auto frontAABB = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
				const auto backAABB  = Geometry::AABB(glm::vec3(-1.f), glm::vec3(1.f));
			}
		}
	}

	void GeometryTester::runTriangleTests()
	{
		const auto control = Geometry::Triangle(glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f));

		{ // Transform tests
			{ // Identity
				auto transform = glm::identity<glm::mat4>();
				auto t1 = control;
				t1.transform(transform);
				runUnitTest({t1 == control, "Triangle - Transform - Identity", "Expected identity transform matrix to not change triangle"});
			}
			{ // Transform - Translate
				auto transformed = control;
				const glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(3.f, 0.f, 0.f)); // Keep translating right

				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(3.f, 1.f, 0.f), glm::vec3(4.f, -1.f, 0.f), glm::vec3(2.f, -1.f, 0.f));
					runUnitTest({transformed == expected, "Triangle - Transform - Translate right 1", "Not matching expected result"});
				}
				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(6.f, 1.f, 0.f), glm::vec3(7.f, -1.f, 0.f), glm::vec3(5.f, -1.f, 0.f));
					runUnitTest({transformed == expected, "Triangle - Transform - Translate right 2", "Not matching expected result"});
				}
				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(9.f, 1.f, 0.f), glm::vec3(10.f, -1.f, 0.f), glm::vec3(8.f, -1.f, 0.f));
					runUnitTest({transformed == expected, "Triangle - Transform - Translate right 3", "Not matching expected result"});
				}
				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(12.f, 1.f, 0.f), glm::vec3(13.f, -1.f, 0.f), glm::vec3(11.f, -1.f, 0.f));
					runUnitTest({transformed == expected, "Triangle - Transform - Translate right 4", "Not matching expected result"});
				}
			}
			{ // Transform - Rotate
				auto transformed = control;
				const glm::mat4 transform = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));

				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(0.f, -0.33333337f, 1.3333334f), glm::vec3(0.99999994f, -0.3333333f, -0.6666666f), glm::vec3(-0.99999994f, -0.3333333f, -0.6666666f));
					runUnitTest({transformed == expected, "Triangle - Transform - Translate right 1", "Not matching expected result"});
				}
				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(0.f, -1.6666667f, -5.9604645e-08f), glm::vec3(0.9999999f, 0.3333333f, 8.940697e-08f), glm::vec3(-0.9999999f, 0.3333333f, 8.940697e-08f));
					runUnitTest({transformed == expected, "Triangle - Transform - Translate right 2", "Not matching expected result"});
				}
				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(0.f, -0.33333325f, -1.3333333f), glm::vec3(0.9999998f, -0.33333346f, 0.66666675f), glm::vec3(-0.9999998f, -0.33333346f, 0.66666675f));
					runUnitTest({transformed == expected, "Triangle - Transform - Translate right 3", "Not matching expected result"});
				}
				{
					transformed.transform(transform);
					auto expected = Geometry::Triangle(glm::vec3(0.f, 0.9999999f, 2.9802322e-07f), glm::vec3(0.99999976f, -1.0000001f, 0.f), glm::vec3(-0.99999976f, -1.0000001f, 0.f));
					runUnitTest({transformed == expected, "Triangle - Transform - Translate right 4", "Not matching expected result"});
				}
			}

			{ // Transform - Scale
				//transform = glm::scale(transform, glm::vec3(2.0f));
			}
			{ // Transform - Combined

			}
		}

		{// No Collision / Coplanar
			auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.5f, 0.f), glm::vec3(1.f, 1.5f, 0.f), glm::vec3(-1.f, 1.5f, 0.f));
			auto t2 = Geometry::Triangle(glm::vec3(0.f, -1.5f, 0.f), glm::vec3(1.f, -3.5f, 0.f), glm::vec3(-1.f, -3.5f, 0.f));
			auto t3 = Geometry::Triangle(glm::vec3(-2.5f, 1.f, 0.f), glm::vec3(-1.5f, -1.f, 0.f), glm::vec3(-3.5f, -1.f, 0.f));
			auto t4 = Geometry::Triangle(glm::vec3(2.5f, 1.f, 0.f), glm::vec3(3.5f, -1.f, 0.f), glm::vec3(1.5f, -1.f, 0.f));
			auto t5 = Geometry::Triangle(glm::vec3(0.f, 1.f, 1.f), glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f));
			auto t6 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(1.f, -1.f, -1.f), glm::vec3(-1.f, -1.f, -1.f));

			runUnitTest({!Geometry::intersecting(control, t1), "Triangle v Triangle - Coplanar - no collision 1", "Expected no collision"});
			runUnitTest({!Geometry::intersecting(control, t2), "Triangle v Triangle - Coplanar - no collision 2", "Expected no collision"});
			runUnitTest({!Geometry::intersecting(control, t3), "Triangle v Triangle - Coplanar - no collision 3", "Expected no collision"});
			runUnitTest({!Geometry::intersecting(control, t4), "Triangle v Triangle - Coplanar - no collision 4", "Expected no collision"});
			runUnitTest({!Geometry::intersecting(control, t5), "Triangle v Triangle - Coplanar - no collision 5", "Expected no collision"});
			runUnitTest({!Geometry::intersecting(control, t6), "Triangle v Triangle - Coplanar - no collision 6", "Expected no collision"});
		}
		{// Collision = true / Coplanar / edge-edge
			auto t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(-2.f, 1.f, 0.f));
			auto t2 = Geometry::Triangle(glm::vec3(1.f, 3.f, 0.f), glm::vec3(2.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
			auto t3 = Geometry::Triangle(glm::vec3(-2.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-3.f, -1.f, 0.f));
			auto t4 = Geometry::Triangle(glm::vec3(2.f, 1.f, 0.f), glm::vec3(3.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f));
			auto t5 = Geometry::Triangle(glm::vec3(-1.f, -1.f, 0.f), glm::vec3(0.f, -3.f, 0.f), glm::vec3(-2.f, -3.f, 0.f));
			auto t6 = Geometry::Triangle(glm::vec3(1.f, -1.f, 0.f), glm::vec3(2.f, -3.f, 0.f), glm::vec3(0.f, -3.f, 0.f));

			runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Coplanar - edge-edge 1", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - Coplanar - edge-edge 2", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - Coplanar - edge-edge 3", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - Coplanar - edge-edge 4", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - Coplanar - edge-edge 5", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - Coplanar - edge-edge 6", "Expected collision to be true"});
		}
		{// Collision = true / non-coplanar / edge-edge
			auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 2.f));
			auto t2 = Geometry::Triangle(glm::vec3(0.f, 3.f, -1.f), glm::vec3(0.f, 1.f, -2.f), glm::vec3(0.f, 1.f, 0.f));
			auto t3 = Geometry::Triangle(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 2.f));
			auto t4 = Geometry::Triangle(glm::vec3(1.f, 1.f, -1.f), glm::vec3(1.f, -1.f, -2.f), glm::vec3(1.f, -1.f, 0.f));
			auto t5 = Geometry::Triangle(glm::vec3(-1.f, 1.f, 1.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(-1.f, -1.f, 2.f));
			auto t6 = Geometry::Triangle(glm::vec3(-1.f, 1.f, -1.f), glm::vec3(-1.f, -1.f, -2.f), glm::vec3(-1.f, -1.f, 0.f));

			runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Non-coplanar - edge-edge 1", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - Non-coplanar - edge-edge 2", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - Non-coplanar - edge-edge 3", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - Non-coplanar - edge-edge 4", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - Non-coplanar - edge-edge 5", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - Non-coplanar - edge-edge 6", "Expected collision to be true"});
		}
		{// Collision = true / coplanar / edge-side
			auto t1 = Geometry::Triangle(glm::vec3(0.f, 3.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(-1.f, 1.f, 0.f));
			auto t2 = Geometry::Triangle(glm::vec3(1.5, 2.f, 0.f), glm::vec3(2.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f));
			auto t3 = Geometry::Triangle(glm::vec3(1.5f, 0.f, 0.f), glm::vec3(2.5f, -2.f, 0.f), glm::vec3(0.5f, -2.f, 0.f));
			auto t4 = Geometry::Triangle(glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f, -3.f, 0.f), glm::vec3(-1.f, -3.f, 0.f));
			auto t5 = Geometry::Triangle(glm::vec3(-1.5f, 0.f, 0.f), glm::vec3(-0.5f, -2.f, 0.f), glm::vec3(-2.5f, -2.f, 0.f));
			auto t6 = Geometry::Triangle(glm::vec3(-1.5f, 2.f, 0.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-2.5f, 0.f, 0.f));

			runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Coplanar - edge-side 1", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - Coplanar - edge-side 2", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - Coplanar - edge-side 3", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - Coplanar - edge-side 4", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - Coplanar - edge-side 5", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - Coplanar - edge-side 6", "Expected collision to be true"});
		}
		{// Collision = true / Non-coplanar / edge-side
			auto t1 = Geometry::Triangle(glm::vec3(0.5f, 2.f, 1.f), glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 2.f));
			auto t2 = Geometry::Triangle(glm::vec3(0.5f, 2.f, -1.f), glm::vec3(0.5f, 0.f, -2.f), glm::vec3(0.5f, 0.f, 0.f));
			auto t3 = Geometry::Triangle(glm::vec3(0.f, 1.f, 1.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 2.f));
			auto t4 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(0.f, -1.f, -2.f), glm::vec3(0.f, -1.f, 0.f));
			auto t5 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, 1.f), glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(-0.5f, 0.f, 2.f));
			auto t6 = Geometry::Triangle(glm::vec3(-0.5f, 2.f, -1.f), glm::vec3(-0.5f, 0.f, -2.f), glm::vec3(-0.5f, 0.f, 0.f));

			runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Non-coplanar - edge-side 1", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - Non-coplanar - edge-side 2", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - Non-coplanar - edge-side 3", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - Non-coplanar - edge-side 4", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - Non-coplanar - edge-side 5", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - Non-coplanar - edge-side 6", "Expected collision to be true"});
		}
		{// Collision = true / coplanar / overlap
			auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.5f, 0.f), glm::vec3(1.f, 0.5f, 0.f), glm::vec3(-1.f, 0.5f, 0.f));
			auto t2 = Geometry::Triangle(glm::vec3(1.f, 2.f, 0.f), glm::vec3(2.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f));
			auto t3 = Geometry::Triangle(glm::vec3(1.f, 0.f, 0.f), glm::vec3(2.f, -2.f, 0.f), glm::vec3(0.f, -2.f, 0.f));
			auto t4 = Geometry::Triangle(glm::vec3(0.f, -0.5f, 0.f), glm::vec3(1.f, -2.5f, 0.f), glm::vec3(-1.f, -2.5f, 0.f));
			auto t5 = Geometry::Triangle(glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -2.f, 0.f), glm::vec3(-2.f, -2.f, 0.f));
			auto t6 = Geometry::Triangle(glm::vec3(-1.f, 2.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(-2.f, 0.f, 0.f));

			runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - coplanar - overlap 1", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - coplanar - overlap 2", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t3), "Triangle v Triangle - coplanar - overlap 3", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t4), "Triangle v Triangle - coplanar - overlap 4", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t5), "Triangle v Triangle - coplanar - overlap 5", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t6), "Triangle v Triangle - coplanar - overlap 6", "Expected collision to be true"});
		}
		{// Collision = true / non-coplanar / overlap
			auto t1 = Geometry::Triangle(glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 0.f, 1.f));
			auto t2 = Geometry::Triangle(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -2.f, -1.f), glm::vec3(0.f, -2.f, 1.f));
			runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - non-coplanar - overlap 1", "Expected collision to be true"});
			runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - non-coplanar - overlap 2", "Expected collision to be true"});
		}
		{// Collision - off-axis collisions
			auto t1 = Geometry::Triangle(glm::vec3(2.f, 1.f, -1.f), glm::vec3(1.f, -2.f, 1.f), glm::vec3(-1.f, -2.f, 1.f));
			runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - off-axis - one side collision", "Expected collision to be true"});

			// Like t1 but two sides of triangle cut through control
			auto t2 = Geometry::Triangle(glm::vec3(0.f, 2.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
			runUnitTest({Geometry::intersecting(control, t2), "Triangle v Triangle - off-axis - two side collision", "Expected collision to be true"});

			// Triangle passes under control without collision
			auto t3 = Geometry::Triangle(glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
			runUnitTest({!Geometry::intersecting(control, t3), "Triangle v Triangle - off-axis - pass under no collision", "Expected no collision"});
		}
		{ // Epsilon tests
			// Place test triangles touching control then move them away by epsilon and check no collision.
			{ // Coplanar to control touching edge to edge
				// t1 bottom-right edge touches the control top edge
				auto t1 = Geometry::Triangle(glm::vec3(-1.f, 3.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(-2.f, 1.f, 0.f));
				runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon co-planar edge-edge", "Expected collision to be true"});
				t1.translate({-std::numeric_limits<float>::epsilon() * 2.f, 0.f, 0.f});
				runUnitTest({!Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon co-planar edge-edge", "Expected no collision after moving left"});
			}
			{
				// Perpendicular to control (non-coplanar), touching the bottom.
				auto t1 = Geometry::Triangle(glm::vec3(0.f, -1.f, -1.f), glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f));
				runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon perpendicular", "Expected collision to be true"});
				t1.translate({0.f, -std::numeric_limits<float>::epsilon(), 0.f});
				runUnitTest({!Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon perpendicular", "Expected no collision after moving down"});
			}
			{
				// Triangle passes under control touching the bottom side at an angle
				auto t1 = Geometry::Triangle(glm::vec3(0.f, 1.f, -1.f), glm::vec3(1.f, -3.f, 1.f), glm::vec3(-1.f, -3.f, 1.f));
				runUnitTest({Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon off-axis - pass under touch", "Expected collision to be true"});
				// Triangle moved below control by epsilon to no longer collide
				t1.translate({0.f, -std::numeric_limits<float>::epsilon(), 0.f});
				runUnitTest({!Geometry::intersecting(control, t1), "Triangle v Triangle - Epsilon off-axis - pass under epsilon distance", "Expected no collision after moving down"});
			}
		}
		{// Edge cases
			runUnitTest({Geometry::intersecting(control, control), "Triangle v Triangle - equal triangles", "Expected collision to be true"});
		}
	}

	void GeometryTester::run_frustrum_tests()
	{
		{ // Create an 'identity' ortho projection and check the planes resulting.
			float ortho_size = 1.f;
			float near       = -1.f;
			float far        = 1.f;
			auto projection  = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near, far);
			auto frustrum    = Geometry::Frustrum(projection);

			runUnitTest({frustrum.m_left.m_distance   == ortho_size, "Frustrum from ortho projection identity - distance - left", "Distance should match the ortho size"});
			runUnitTest({frustrum.m_right.m_distance  == ortho_size, "Frustrum from ortho projection identity - distance - right", "Distance should match the ortho size"});
			runUnitTest({frustrum.m_bottom.m_distance == ortho_size, "Frustrum from ortho projection identity - distance - bottom", "Distance should match the ortho size"});
			runUnitTest({frustrum.m_top.m_distance    == ortho_size, "Frustrum from ortho projection identity - distance - top", "Distance should match the ortho size"});
			runUnitTest({frustrum.m_near.m_distance   == -1.f, "Frustrum from ortho projection identity - distance - near", "Distance should match the near size"});
			runUnitTest({frustrum.m_far.m_distance    == -1.f, "Frustrum from ortho projection identity - distance - far", "Distance should match the far size"});

			runUnitTest({frustrum.m_left.m_normal   == glm::vec3(1.f, 0.f, 0.f), "Frustrum from ortho projection identity - normal - left", "Should be pointing towards the negative x-axis"});
			runUnitTest({frustrum.m_right.m_normal  == glm::vec3(-1.f, 0.f, 0.f), "Frustrum from ortho projection identity - normal - right", "Should be pointing towards the positive x-axis"});
			runUnitTest({frustrum.m_bottom.m_normal == glm::vec3(0.f, 1.f, 0.f), "Frustrum from ortho projection identity - normal - bottom", "Should be pointing towards the negative y-axis"});
			runUnitTest({frustrum.m_top.m_normal    == glm::vec3(0.f, -1.f, 0.f), "Frustrum from ortho projection identity - normal - top", "Should be pointing towards the positive y-axis"});
			runUnitTest({frustrum.m_near.m_normal   == glm::vec3(0.f, 0.f, 1.f), "Frustrum from ortho projection identity - normal - near", "Should be pointing towards the negative z-axis"});
			runUnitTest({frustrum.m_far.m_normal    == glm::vec3(0.f, 0.f, -1.f), "Frustrum from ortho projection identity - normal - far", "Should be pointing towards the positive z-axis"});
		}
		{ // Create an 'non-identity' ortho projection and check the planes resulting. Previous test can get away with non-normalising of the plane equations, but this test uses a non-1 ortho_size.
			float ortho_size = 15.f;
			float near       = 0.f;
			float far        = 10.f;
			auto projection  = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near, far);
			auto frustrum    = Geometry::Frustrum(projection);

			auto error_threshold_equality = [](float value_1, float value_2, float threshold = std::numeric_limits<float>::epsilon(), float power = 0.f)
			{
				auto adjusted_threshold = threshold * std::pow(10.f, power);
				return std::abs(value_1 - value_2) <= adjusted_threshold;
			};

			runUnitTest({error_threshold_equality(frustrum.m_left.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - left", "Distance should match the ortho size"});
			runUnitTest({error_threshold_equality(frustrum.m_left.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - left", "Distance should match the ortho size"});
			runUnitTest({error_threshold_equality(frustrum.m_right.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - right", "Distance should match the ortho size"});
			runUnitTest({error_threshold_equality(frustrum.m_bottom.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - bottom", "Distance should match the ortho size"});
			runUnitTest({error_threshold_equality(frustrum.m_top.m_distance, ortho_size, std::numeric_limits<float>::epsilon(), 1.f), "Frustrum from ortho projection - distance - top", "Distance should match the ortho size"});
			runUnitTest({frustrum.m_near.m_distance   == 0.f, "Frustrum from ortho projection - distance - near", "Distance should match the near size"});
			runUnitTest({frustrum.m_far.m_distance    == -10.f, "Frustrum from ortho projection - distance - far", "Distance should match the far size"});

			runUnitTest({frustrum.m_left.m_normal   == glm::vec3(1.f, 0.f, 0.f), "Frustrum from ortho projection - normal - left", "Should be pointing towards the negative x-axis"});
			runUnitTest({frustrum.m_right.m_normal  == glm::vec3(-1.f, 0.f, 0.f), "Frustrum from ortho projection - normal - right", "Should be pointing towards the positive x-axis"});
			runUnitTest({frustrum.m_bottom.m_normal == glm::vec3(0.f, 1.f, 0.f), "Frustrum from ortho projection - normal - bottom", "Should be pointing towards the negative y-axis"});
			runUnitTest({frustrum.m_top.m_normal    == glm::vec3(0.f, -1.f, 0.f), "Frustrum from ortho projection - normal - top", "Should be pointing towards the positive y-axis"});
			runUnitTest({frustrum.m_near.m_normal   == glm::vec3(0.f, 0.f, 1.f), "Frustrum from ortho projection - normal - near", "Should be pointing towards the negative z-axis"});
			runUnitTest({frustrum.m_far.m_normal    == glm::vec3(0.f, 0.f, -1.f), "Frustrum from ortho projection - normal - far", "Should be pointing towards the positive z-axis"});
		}
	}

	void GeometryTester::run_sphere_tests()
	{
			{ // Touching (point collision)
				auto sphere   = Geometry::Sphere(glm::vec3(0.f, 0.f, 0.f), 1.f);
				auto sphere_2 = Geometry::Sphere(glm::vec3(2.f, 0.f, 0.f), 1.f);

				auto intersecting = Geometry::intersecting(sphere, sphere_2);
				CHECK_TRUE(intersecting, "Spheres touching intersecting test");

				auto intersection = Geometry::get_intersection(sphere, sphere_2);
				CHECK_TRUE(intersection.has_value(), "Spheres touching intersection test");

				if (intersection.has_value())
				{// Touching returns a LineSegment with the same start and end point.
					CHECK_EQUAL(intersection->m_start, glm::vec3(1.f, 0.f, 0.f), "Intersection start  - Spheres touching");
					CHECK_EQUAL(intersection->m_end,   glm::vec3(1.f, 0.f, 0.f), "Intersection end    - Spheres touching");
					CHECK_EQUAL(intersection->length(), 0.f,                     "Intersection length - Spheres touching");
				}
			}
			{ // Overlapping (line collision)
				auto sphere   = Geometry::Sphere(glm::vec3(0.f, 0.f, 0.f), 1.25f);
				auto sphere_2 = Geometry::Sphere(glm::vec3(2.f, 0.f, 0.f), 1.25f);

				auto intersecting = Geometry::intersecting(sphere, sphere_2);
				CHECK_TRUE(intersecting, "Spheres overlapping intersecting test");

				auto intersection = Geometry::get_intersection(sphere, sphere_2);
				CHECK_TRUE(intersection.has_value(), "Spheres overlapping intersection test");

				if (intersection.has_value())
				{// Touching returns a LineSegment with the same start and end point.
					CHECK_EQUAL(intersection->m_start, glm::vec3(0.75f, 0.f, 0.f),   "Intersection start     - Spheres overlapping");
					CHECK_EQUAL(intersection->m_end,   glm::vec3(1.25f, 0.f, 0.f),   "Intersection end       - Spheres overlapping");
					CHECK_EQUAL(intersection->length(), 0.5f,                        "Intersection length    - Spheres overlapping");
					CHECK_EQUAL(intersection->direction(), glm::vec3(1.f, 0.f, 0.f), "Intersection direction - Spheres overlapping");
				}
			}
			{ // Not intersecting
				auto sphere   = Geometry::Sphere(glm::vec3(0.f, 0.f, 0.f), 0.5f);
				auto sphere_2 = Geometry::Sphere(glm::vec3(2.f, 0.f, 0.f), 0.5f);

				auto intersecting = Geometry::intersecting(sphere, sphere_2);
				CHECK_TRUE(!intersecting, "Spheres not intersecting - intersecting test");

				auto intersection = Geometry::get_intersection(sphere, sphere_2);
				CHECK_TRUE(!intersection.has_value(), "Spheres not intersecting - intersection test");
			}
			{ // Not intersecting epsilon - reduce the size of one of the spheres touching by epsilon, should not intersect anymore
				auto sphere   = Geometry::Sphere(glm::vec3(0.f, 0.f, 0.f), 1.f - std::numeric_limits<float>::epsilon());
				auto sphere_2 = Geometry::Sphere(glm::vec3(2.f, 0.f, 0.f), 1.f);

				auto intersecting = Geometry::intersecting(sphere, sphere_2);
				CHECK_TRUE(!intersecting, "Spheres not intersecting epsilon - intersecting test");

				auto intersection = Geometry::get_intersection(sphere, sphere_2);
				CHECK_TRUE(!intersection.has_value(), "Spheres not intersecting epsilon - intersection test");
			}
	}

	void GeometryTester::run_point_tests()
	{
		{// Point v AABB
			const auto AABB = Geometry::AABB(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f, 1.f, 1.f));

			// Test a point inside the AABB
			{
				const auto point_inside = Geometry::Point(glm::vec3(0.f, 0.f, 0.f));

				auto inside = Geometry::intersecting(point_inside, AABB);
				CHECK_TRUE(inside, "Point inside AABB");
				CHECK_EQUAL(Geometry::intersecting(point_inside, AABB), Geometry::intersecting(AABB, point_inside), "Point inside AABB overload");

				auto intersection = Geometry::get_intersection(point_inside, AABB);
				CHECK_TRUE(intersection.has_value(), "Point inside AABB intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_inside, AABB), Geometry::get_intersection(AABB, point_inside), "Point inside AABB intersection overload");

				if (intersection.has_value())
				{
					CHECK_EQUAL(intersection->m_position, point_inside.m_position, "Point inside AABB intersection position");
				}
			}
			{// Test a point on the surface of AABB
				const auto point_on_surface = Geometry::Point(glm::vec3(1.f, 1.f, 1.f));

				auto on_surface = Geometry::intersecting(point_on_surface, AABB);
				CHECK_TRUE(on_surface, "Point on surface of AABB");
				CHECK_EQUAL(Geometry::intersecting(point_on_surface, AABB), Geometry::intersecting(AABB, point_on_surface), "Point on surface of AABB overload");

				auto intersection = Geometry::get_intersection(point_on_surface, AABB);
				CHECK_TRUE(intersection.has_value(), "Point on surface of AABB intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_surface, AABB), Geometry::get_intersection(AABB, point_on_surface), "Point on surface of AABB intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_surface.m_position, "Point on surface of AABB intersection position");
			}
			{// Test a point outside the AABB
				const auto point_outside = Geometry::Point(glm::vec3(2.f, 0.f, 2.f));

				auto outside = Geometry::intersecting(point_outside, AABB);
				CHECK_TRUE(!outside, "Point outside AABB");
				CHECK_EQUAL(Geometry::intersecting(point_outside, AABB), Geometry::intersecting(AABB, point_outside), "Point outside AABB overload");

				auto intersection = Geometry::get_intersection(point_outside, AABB);
				CHECK_TRUE(!intersection.has_value(), "Point outside AABB intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_outside, AABB), Geometry::get_intersection(AABB, point_outside), "Point outside AABB intersection overload");
			}
			{// Test a point on the max edge of the AABB
				const auto point_on_max_edge = Geometry::Point(glm::vec3(1.f, 1.f, 1.f));

				auto on_max_edge = Geometry::intersecting(point_on_max_edge, AABB);
				CHECK_TRUE(on_max_edge, "Point on max edge of AABB");
				CHECK_EQUAL(Geometry::intersecting(point_on_max_edge, AABB), Geometry::intersecting(AABB, point_on_max_edge), "Point on max edge of AABB overload");

				auto intersection = Geometry::get_intersection(point_on_max_edge, AABB);
				CHECK_TRUE(intersection.has_value(), "Point on max edge of AABB intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_max_edge, AABB), Geometry::get_intersection(AABB, point_on_max_edge), "Point on max edge of AABB intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_max_edge.m_position, "Point on max edge of AABB intersection position");
			}
			{// Test a point on the min edge of the AABB
				const auto point_on_min_edge = Geometry::Point(glm::vec3(-1.f, -1.f, -1.f));

				auto on_min_edge = Geometry::intersecting(point_on_min_edge, AABB);
				CHECK_TRUE(on_min_edge, "Point on min edge of AABB");
				CHECK_EQUAL(Geometry::intersecting(point_on_min_edge, AABB), Geometry::intersecting(AABB, point_on_min_edge), "Point on min edge of AABB overload");

				auto intersection = Geometry::get_intersection(point_on_min_edge, AABB);
				CHECK_TRUE(intersection.has_value(), "Point on min edge of AABB intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_min_edge, AABB), Geometry::get_intersection(AABB, point_on_min_edge), "Point on min edge of AABB intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_min_edge.m_position, "Point on min edge of AABB intersection position");
			}
		}
		{// Point v Cone
			const auto cone = Geometry::Cone(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), 1.f);

			{// Test point inside the cone
				auto point_inside = Geometry::Point(glm::vec3(0.f, 0.5f, 0.f));

				auto inside = Geometry::intersecting(point_inside, cone);
				CHECK_TRUE(inside, "Point inside cone");
				CHECK_EQUAL(Geometry::intersecting(point_inside, cone), Geometry::intersecting(cone, point_inside), "Point inside cone overload");

				auto intersection = Geometry::get_intersection(point_inside, cone);
				CHECK_TRUE(intersection.has_value(), "Point inside cone intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_inside, cone), Geometry::get_intersection(cone, point_inside), "Point inside cone intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_inside.m_position, "Point inside cone intersection position");
			}
			{// Test point outside the cone
				auto point_outside = Geometry::Point(glm::vec3(0.f, 1.5f, 0.f));

				auto outside = Geometry::intersecting(point_outside, cone);
				CHECK_TRUE(!outside, "Point outside cone");
				CHECK_EQUAL(Geometry::intersecting(point_outside, cone), Geometry::intersecting(cone, point_outside), "Point outside cone overload");

				auto intersection = Geometry::get_intersection(point_outside, cone);
				CHECK_TRUE(!intersection.has_value(), "Point outside cone intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_outside, cone), Geometry::get_intersection(cone, point_outside), "Point outside cone intersection overload");
			}
			{// Test point on the cone surface (top)
				auto point_on_surface = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));

				auto on_surface = Geometry::intersecting(point_on_surface, cone);
				CHECK_TRUE(on_surface, "Point on surface of cone (top)");
				CHECK_EQUAL(Geometry::intersecting(point_on_surface, cone), Geometry::intersecting(cone, point_on_surface), "Point on surface of cone (top) overload");

				auto intersection = Geometry::get_intersection(point_on_surface, cone);
				CHECK_TRUE(intersection.has_value(), "Point on surface of cone (top) intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_surface, cone), Geometry::get_intersection(cone, point_on_surface), "Point on surface of cone (top) intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_surface.m_position, "Point on surface of cone (top) intersection position");
			}
			{// Test point on the cone surface (base)
				auto point_on_surface = Geometry::Point(glm::vec3(0.f));

				auto on_surface = Geometry::intersecting(point_on_surface, cone);
				CHECK_TRUE(on_surface, "Point on surface of cone (base)");
				CHECK_EQUAL(Geometry::intersecting(point_on_surface, cone), Geometry::intersecting(cone, point_on_surface), "Point on surface of cone (base) overload");

				auto intersection = Geometry::get_intersection(point_on_surface, cone);
				CHECK_TRUE(intersection.has_value(), "Point on surface of cone (base) intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_surface, cone), Geometry::get_intersection(cone, point_on_surface), "Point on surface of cone (base) intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_surface.m_position, "Point on surface of cone (base) intersection position");
			}
			{// Test point on the cone surface (side)
				auto point_on_surface = Geometry::Point(glm::vec3(0.f, 0.5f, 0.5f));

				auto on_surface = Geometry::intersecting(point_on_surface, cone);
				CHECK_TRUE(on_surface, "Point on surface of cone (side)");
				CHECK_EQUAL(Geometry::intersecting(point_on_surface, cone), Geometry::intersecting(cone, point_on_surface), "Point on surface of cone (side) overload");

				auto intersection = Geometry::get_intersection(point_on_surface, cone);
				CHECK_TRUE(intersection.has_value(), "Point on surface of cone (side) intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_surface, cone), Geometry::get_intersection(cone, point_on_surface), "Point on surface of cone (side) intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_surface.m_position, "Point on surface of cone (side) intersection position");
			}
		}
		{// Point v Cylinder
			const auto cylinder = Geometry::Cylinder(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), 1.f);

			{// Test point inside the cylinder
				auto point_inside = Geometry::Point(glm::vec3(0.5f, 0.5f, 0.5f));

				auto inside = Geometry::intersecting(point_inside, cylinder);
				CHECK_TRUE(inside, "Point inside cylinder");
				CHECK_EQUAL(Geometry::intersecting(point_inside, cylinder), Geometry::intersecting(cylinder, point_inside), "Point inside cylinder overload");

				auto intersection = Geometry::get_intersection(point_inside, cylinder);
				CHECK_TRUE(intersection.has_value(), "Point inside cylinder intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_inside, cylinder), Geometry::get_intersection(cylinder, point_inside), "Point inside cylinder intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_inside.m_position, "Point inside cylinder intersection position");
			}
			{// Test point outside the cylinder
				auto point_outside = Geometry::Point(glm::vec3(0.5f, 1.5f, 0.5f));

				auto outside = Geometry::intersecting(point_outside, cylinder);
				CHECK_TRUE(!outside, "Point outside cylinder");
				CHECK_EQUAL(Geometry::intersecting(point_outside, cylinder), Geometry::intersecting(cylinder, point_outside), "Point outside cylinder overload");

				auto intersection = Geometry::get_intersection(point_outside, cylinder);
				CHECK_TRUE(!intersection.has_value(), "Point outside cylinder intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_outside, cylinder), Geometry::get_intersection(cylinder, point_outside), "Point outside cylinder intersection overload");
			}
			{// Test point on the cylinder surface (top)
				auto point_on_surface = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));

				auto on_surface = Geometry::intersecting(point_on_surface, cylinder);
				CHECK_TRUE(on_surface, "Point on surface of cylinder (top)");
				CHECK_EQUAL(Geometry::intersecting(point_on_surface, cylinder), Geometry::intersecting(cylinder, point_on_surface), "Point on surface of cylinder (top) overload");

				auto intersection = Geometry::get_intersection(point_on_surface, cylinder);
				CHECK_TRUE(intersection.has_value(), "Point on surface of cylinder (top) intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_surface, cylinder), Geometry::get_intersection(cylinder, point_on_surface), "Point on surface of cylinder (top) intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_surface.m_position, "Point on surface of cylinder (top) intersection position");
			}
			{// Test point on the cylinder surface (base)
				auto point_on_surface = Geometry::Point(glm::vec3(0.f));

				auto on_surface = Geometry::intersecting(point_on_surface, cylinder);
				CHECK_TRUE(on_surface, "Point on surface of cylinder (base)");
				CHECK_EQUAL(Geometry::intersecting(point_on_surface, cylinder), Geometry::intersecting(cylinder, point_on_surface), "Point on surface of cylinder (base) overload");

				auto intersection = Geometry::get_intersection(point_on_surface, cylinder);
				CHECK_TRUE(intersection.has_value(), "Point on surface of cylinder (base) intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_surface, cylinder), Geometry::get_intersection(cylinder, point_on_surface), "Point on surface of cylinder (base) intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_surface.m_position, "Point on surface of cylinder (base) intersection position");
			}
			{// Test point on the cylinder surface (side)
				auto point_on_surface = Geometry::Point(glm::vec3(0.f, 0.5f, 0.5f));

				auto on_surface = Geometry::intersecting(point_on_surface, cylinder);
				CHECK_TRUE(on_surface, "Point on surface of cylinder (side)");
				CHECK_EQUAL(Geometry::intersecting(point_on_surface, cylinder), Geometry::intersecting(cylinder, point_on_surface), "Point on surface of cylinder (side) overload");

				auto intersection = Geometry::get_intersection(point_on_surface, cylinder);
				CHECK_TRUE(intersection.has_value(), "Point on surface of cylinder (side) intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_surface, cylinder), Geometry::get_intersection(cylinder, point_on_surface), "Point on surface of cylinder (side) intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_surface.m_position, "Point on surface of cylinder (side) intersection position");
			}
		}// Point v Cylinder
		{// Point v Line
			const auto line = Geometry::Line(glm::vec3(-1.f), glm::vec3(1.f));

			{// Point in middle of line
				auto point_on_line_middle = Geometry::Point(glm::vec3(0.f));

				auto on_line = Geometry::intersecting(point_on_line_middle, line);
				CHECK_TRUE(on_line, "Point on line");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_middle, line), Geometry::intersecting(line, point_on_line_middle), "Point on line overload");

				auto intersection = Geometry::get_intersection(point_on_line_middle, line);
				CHECK_TRUE(intersection.has_value(), "Point on line intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_middle, line), Geometry::get_intersection(line, point_on_line_middle), "Point on line intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_line_middle.m_position, "Point on line intersection position");
			}
			{// Point on start of line
				auto point_on_line_start = Geometry::Point(glm::vec3(-1.f));

				auto on_line = Geometry::intersecting(point_on_line_start, line);
				CHECK_TRUE(on_line, "Point at start of line");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_start, line), Geometry::intersecting(line, point_on_line_start), "Point at start of line overload");

				auto intersection = Geometry::get_intersection(point_on_line_start, line);
				CHECK_TRUE(intersection.has_value(), "Point at start of line intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_start, line), Geometry::get_intersection(line, point_on_line_start), "Point at start of line intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_line_start.m_position, "Point at start of line intersection position");
			}
			{// Point on end of line
				auto point_on_line_end = Geometry::Point(glm::vec3(1.f));

				auto on_line = Geometry::intersecting(point_on_line_end, line);
				CHECK_TRUE(on_line, "Point at end of line");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_end, line), Geometry::intersecting(line, point_on_line_end), "Point at end of line overload");

				auto intersection = Geometry::get_intersection(point_on_line_end, line);
				CHECK_TRUE(intersection.has_value(), "Point at end of line intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_end, line), Geometry::get_intersection(line, point_on_line_end), "Point at end of line intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_line_end.m_position, "Point at end of line intersection position");
			}
			{// Point off line
				auto point_off_line_above = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));

				auto on_line = Geometry::intersecting(point_off_line_above, line);
				CHECK_TRUE(!on_line, "Point off line");
				CHECK_EQUAL(Geometry::intersecting(point_off_line_above, line), Geometry::intersecting(line, point_off_line_above), "Point off line overload");

				auto intersection = Geometry::get_intersection(point_off_line_above, line);
				CHECK_TRUE(!intersection.has_value(), "Point off line intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_off_line_above, line), Geometry::get_intersection(line, point_off_line_above), "Point off line intersection overload");
			}
			{// Point on line ahead points used to construct line
				auto point_on_line_ahead = Geometry::Point(glm::vec3(2.f));

				auto on_line = Geometry::intersecting(point_on_line_ahead, line);
				CHECK_TRUE(on_line, "Point on line ahead");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_ahead, line), Geometry::intersecting(line, point_on_line_ahead), "Point on line ahead overload");

				auto intersection = Geometry::get_intersection(point_on_line_ahead, line);
				CHECK_TRUE(intersection.has_value(), "Point on line ahead intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_ahead, line), Geometry::get_intersection(line, point_on_line_ahead), "Point on line ahead intersection overload");
			}
			{// Point on line behind points used to construct line
				auto point_on_line_behind = Geometry::Point(glm::vec3(-2.f));

				auto on_line = Geometry::intersecting(point_on_line_behind, line);
				CHECK_TRUE(on_line, "Point on line ahead");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_behind, line), Geometry::intersecting(line, point_on_line_behind), "Point on line ahead overload");

				auto intersection = Geometry::get_intersection(point_on_line_behind, line);
				CHECK_TRUE(intersection.has_value(), "Point on line ahead intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_behind, line), Geometry::get_intersection(line, point_on_line_behind), "Point on line ahead intersection overload");
			}
		}
		{// Point v LineSegment
			const auto line_segment = Geometry::LineSegment(glm::vec3(-1.f), glm::vec3(1.f));

			{// Point in middle of line_segment
				auto point_on_line_middle = Geometry::Point(glm::vec3(0.f));

				auto on_line = Geometry::intersecting(point_on_line_middle, line_segment);
				CHECK_TRUE(on_line, "Point on line_segment");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_middle, line_segment), Geometry::intersecting(line_segment, point_on_line_middle), "Point on line_segment overload");

				auto intersection = Geometry::get_intersection(point_on_line_middle, line_segment);
				CHECK_TRUE(intersection.has_value(), "Point on line_segment intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_middle, line_segment), Geometry::get_intersection(line_segment, point_on_line_middle), "Point on line_segment intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_line_middle.m_position, "Point on line_segment intersection position");
			}
			{// Point on start of line_segment
				auto point_on_line_start = Geometry::Point(glm::vec3(-1.f));

				auto on_line = Geometry::intersecting(point_on_line_start, line_segment);
				CHECK_TRUE(on_line, "Point at start of line_segment");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_start, line_segment), Geometry::intersecting(line_segment, point_on_line_start), "Point at start of line_segment overload");

				auto intersection = Geometry::get_intersection(point_on_line_start, line_segment);
				CHECK_TRUE(intersection.has_value(), "Point at start of line_segment intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_start, line_segment), Geometry::get_intersection(line_segment, point_on_line_start), "Point at start of line_segment intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_line_start.m_position, "Point at start of line_segment intersection position");
			}
			{// Point on end of line_segment
				auto point_on_line_end = Geometry::Point(glm::vec3(1.f));

				auto on_line = Geometry::intersecting(point_on_line_end, line_segment);
				CHECK_TRUE(on_line, "Point at end of line_segment");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_end, line_segment), Geometry::intersecting(line_segment, point_on_line_end), "Point at end of line_segment overload");

				auto intersection = Geometry::get_intersection(point_on_line_end, line_segment);
				CHECK_TRUE(intersection.has_value(), "Point at end of line_segment intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_end, line_segment), Geometry::get_intersection(line_segment, point_on_line_end), "Point at end of line_segment intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_line_end.m_position, "Point at end of line_segment intersection position");
			}
			{// Point off line_segment above
				auto point_off_line_above = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));

				auto on_line = Geometry::intersecting(point_off_line_above, line_segment);
				CHECK_TRUE(!on_line, "Point off line_segment");
				CHECK_EQUAL(Geometry::intersecting(point_off_line_above, line_segment), Geometry::intersecting(line_segment, point_off_line_above), "Point off line_segment overload");

				auto intersection = Geometry::get_intersection(point_off_line_above, line_segment);
				CHECK_TRUE(!intersection.has_value(), "Point off line_segment intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_off_line_above, line_segment), Geometry::get_intersection(line_segment, point_off_line_above), "Point off line_segment intersection overload");
			}
			{// Point off line_segment ahead points used to construct line_segment
				auto point_on_line_ahead = Geometry::Point(glm::vec3(2.f));

				auto on_line = Geometry::intersecting(point_on_line_ahead, line_segment);
				CHECK_TRUE(!on_line, "Point off line_segment ahead");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_ahead, line_segment), Geometry::intersecting(line_segment, point_on_line_ahead), "Point off line_segment ahead overload");

				auto intersection = Geometry::get_intersection(point_on_line_ahead, line_segment);
				CHECK_TRUE(!intersection.has_value(), "Point off line_segment ahead intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_ahead, line_segment), Geometry::get_intersection(line_segment, point_on_line_ahead), "Point off line_segment ahead intersection overload");
			}
			{// Point off line_segment behind points used to construct line_segment
				auto point_on_line_behind = Geometry::Point(glm::vec3(-2.f));

				auto on_line = Geometry::intersecting(point_on_line_behind, line_segment);
				CHECK_TRUE(!on_line, "Point off line_segment ahead");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_behind, line_segment), Geometry::intersecting(line_segment, point_on_line_behind), "Point off line_segment ahead overload");

				auto intersection = Geometry::get_intersection(point_on_line_behind, line_segment);
				CHECK_TRUE(!intersection.has_value(), "Point off line_segment ahead intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_behind, line_segment), Geometry::get_intersection(line_segment, point_on_line_behind), "Point off line_segment ahead intersection overload");
			}
		}// Point v LineSegment
		{// Point v Ray
			const auto ray = Geometry::Ray(glm::vec3(-1.f), glm::vec3(1.f));

			{// Point in middle of ray
				auto point_on_line_middle = Geometry::Point(glm::vec3(0.f));

				auto on_line = Geometry::intersecting(point_on_line_middle, ray);
				CHECK_TRUE(on_line, "Point on ray");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_middle, ray), Geometry::intersecting(ray, point_on_line_middle), "Point on ray overload");

				auto intersection = Geometry::get_intersection(point_on_line_middle, ray);
				CHECK_TRUE(intersection.has_value(), "Point on ray intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_middle, ray), Geometry::get_intersection(ray, point_on_line_middle), "Point on ray intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_line_middle.m_position, "Point on ray intersection position");
			}
			{// Point on start of ray
				auto point_on_line_start = Geometry::Point(glm::vec3(-1.f));

				auto on_line = Geometry::intersecting(point_on_line_start, ray);
				CHECK_TRUE(on_line, "Point at start of ray");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_start, ray), Geometry::intersecting(ray, point_on_line_start), "Point at start of ray overload");

				auto intersection = Geometry::get_intersection(point_on_line_start, ray);
				CHECK_TRUE(intersection.has_value(), "Point at start of ray intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_start, ray), Geometry::get_intersection(ray, point_on_line_start), "Point at start of ray intersection overload");

				if (intersection.has_value())
					CHECK_EQUAL(intersection->m_position, point_on_line_start.m_position, "Point at start of ray intersection position");
			}
			{// Point off ray above
				auto point_off_line_above = Geometry::Point(glm::vec3(0.f, 1.f, 0.f));

				auto on_line = Geometry::intersecting(point_off_line_above, ray);
				CHECK_TRUE(!on_line, "Point off ray");
				CHECK_EQUAL(Geometry::intersecting(point_off_line_above, ray), Geometry::intersecting(ray, point_off_line_above), "Point off ray overload");

				auto intersection = Geometry::get_intersection(point_off_line_above, ray);
				CHECK_TRUE(!intersection.has_value(), "Point off ray intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_off_line_above, ray), Geometry::get_intersection(ray, point_off_line_above), "Point off ray intersection overload");
			}
			{// Point off ray ahead points used to construct ray
				auto point_on_line_ahead = Geometry::Point(glm::vec3(2.f));

				auto on_line = Geometry::intersecting(point_on_line_ahead, ray);
				CHECK_TRUE(on_line, "Point off ray ahead");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_ahead, ray), Geometry::intersecting(ray, point_on_line_ahead), "Point off ray ahead overload");

				auto intersection = Geometry::get_intersection(point_on_line_ahead, ray);
				CHECK_TRUE(intersection.has_value(), "Point off ray ahead intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_ahead, ray), Geometry::get_intersection(ray, point_on_line_ahead), "Point off ray ahead intersection overload");
			}
			{// Point off ray behind points used to construct ray
				auto point_on_line_behind = Geometry::Point(glm::vec3(-2.f));

				auto on_line = Geometry::intersecting(point_on_line_behind, ray);
				CHECK_TRUE(!on_line, "Point off ray ahead");
				CHECK_EQUAL(Geometry::intersecting(point_on_line_behind, ray), Geometry::intersecting(ray, point_on_line_behind), "Point off ray ahead overload");

				auto intersection = Geometry::get_intersection(point_on_line_behind, ray);
				CHECK_TRUE(!intersection.has_value(), "Point off ray ahead intersection");
				CHECK_EQUAL(Geometry::get_intersection(point_on_line_behind, ray), Geometry::get_intersection(ray, point_on_line_behind), "Point off ray ahead intersection overload");
			}
		}// Point v Ray
	}

	void GeometryTester::draw_frustrum_debugger_UI(float aspect_ratio)
	{
		// Use this ImGui + OpenGL::DebugRenderer function to visualise Projection generated Geometry::Frustrums.
		// A projection-only generated frustrum is positioned at [0, 0, 0] in the positive-z direction.
		// OpenGL clip coordinates are in the [-1 - 1] range thus the default generate ortho projection has a near = -1, far = 1.
		if (ImGui::Begin("Frustrum visualiser"))
		{
			enum class ProjectionType
			{
				Ortho,
				Perspective
			};
			static const std::vector<std::pair<ProjectionType, const char*>> projection_options =
				{{ProjectionType::Ortho, "Ortho"}, {ProjectionType::Perspective, "Perspective"}};
			static ProjectionType projection_type = ProjectionType::Ortho;
			static float near                     = 0.1f;
			static float far                      = 2.f;
			static float ortho_size               = 1.f;
			static bool use_near_far              = true;
			static float fov                      = 90.f;
			static bool transpose                 = false;
			static bool apply_view                = true;

			ImGui::ComboContainer("Projection type", projection_type, projection_options);

			ImGui::Separator();
			glm::mat4 projection;
			if (projection_type == ProjectionType::Ortho)
			{
				ImGui::Checkbox("use near far", &use_near_far);
				if (use_near_far)
				{
					ImGui::Slider("near", near, -1.f, 20.f);
					ImGui::Slider("far", far, 1.f, 20.f);
				}
				ImGui::Slider("ortho_size", ortho_size, 1.f, 20.f);

				if (use_near_far)
					projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near, far);
				else
					projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size);
			}
			else if (projection_type == ProjectionType::Perspective)
			{
				ImGui::Slider("FOV", fov, 1.f, 180.f);
				ImGui::Slider("Aspect ratio", aspect_ratio, 0.f, 5.f);
				ImGui::Slider("near", near, -1.f, 20.f);
				ImGui::Slider("far", far, 1.f, 20.f);
				projection = glm::perspective(glm::radians(fov), aspect_ratio, near, far);
			}

			ImGui::Separator();
			ImGui::Checkbox("transpose", &transpose);
			if (transpose)
				projection = glm::transpose(projection);

			ImGui::Checkbox("apply view matrix", &apply_view);
			if (apply_view)
			{
				ImGui::Separator();
				static glm::vec3 eye_position = glm::vec3(0.f, 0.f, 0.f);
				static glm::vec3 center       = glm::vec3(0.5f, 0.f, 0.5f);
				static glm::vec3 up           = glm::vec3(0.f, 1.f, 0.f);
				static glm::mat4 view;
				static bool inverse_view     = false;
				static bool transpose_view   = false;
				static bool swap_order       = false;
				static bool flip_view_dir    = true;
				static bool inverse_position = true;

				ImGui::Slider("Position", eye_position, 0.f, 20.f);
				ImGui::Slider("look direction", center, 0.f, 20.f);
				ImGui::Slider("up direction", up, 0.f, 20.f);
				ImGui::Checkbox("Inverse view", &inverse_view);
				ImGui::Checkbox("Transpose view", &transpose_view);
				ImGui::Checkbox("Swap order", &swap_order);
				ImGui::Checkbox("Flip view direction", &flip_view_dir);
				ImGui::Checkbox("inverse position", &inverse_position);

				glm::vec3 view_position = inverse_position ? -eye_position : eye_position;
				glm::vec3 view_look_at  = flip_view_dir ? view_position - center : view_position + center;
				view                    = glm::lookAt(view_position, view_look_at, up);

				if (swap_order)
				{
					if (inverse_view)
						view = glm::inverse(view);
					if (transpose_view)
						view = glm::transpose(view);
				}
				else
				{
					if (transpose_view)
						view = glm::transpose(view);
					if (inverse_view)
						view = glm::inverse(view);
				}
				projection = projection * view;
				ImGui::Text("VIEW", view);
				ImGui::Separator();
			}

			Geometry::Frustrum frustrum = Geometry::Frustrum(projection);
			ImGui::Text("LEFT  \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_left.m_normal.x, frustrum.m_left.m_normal.y, frustrum.m_left.m_normal.z, frustrum.m_left.m_distance);
			ImGui::Text("RIGHT \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_right.m_normal.x, frustrum.m_right.m_normal.y, frustrum.m_right.m_normal.z, frustrum.m_right.m_distance);
			ImGui::Text("BOTTOM\nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_bottom.m_normal.x, frustrum.m_bottom.m_normal.y, frustrum.m_bottom.m_normal.z, frustrum.m_bottom.m_distance);
			ImGui::Text("TOP   \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_top.m_normal.x, frustrum.m_top.m_normal.y, frustrum.m_top.m_normal.z, frustrum.m_top.m_distance);
			ImGui::Text("NEAR  \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_near.m_normal.x, frustrum.m_near.m_normal.y, frustrum.m_near.m_normal.z, frustrum.m_near.m_distance);
			ImGui::Text("FAR   \nNormal: [%.3f, %.3f, %.3f]\nDistance: %.6f\n", frustrum.m_far.m_normal.x, frustrum.m_far.m_normal.y, frustrum.m_far.m_normal.z, frustrum.m_far.m_distance);
			ImGui::Text("PROJECTION", projection);
			OpenGL::DebugRenderer::add(frustrum, glm::vec4(218.f / 255.f, 112.f / 255.f, 214.f / 255.f, 0.5f));
		}
		ImGui::End();
	}
}// namespace Test