#include "software/ai/hl/stp/play/defense_play.h"

#include <gtest/gtest.h>

#include "software/simulated_tests/non_terminating_validation_functions/ball_in_play_or_scored_validation.h"
#include "software/simulated_tests/simulated_play_test_fixture.h"
#include "software/simulated_tests/terminating_validation_functions/robot_halt_validation.h"
#include "software/simulated_tests/terminating_validation_functions/robot_in_polygon_validation.h"
#include "software/simulated_tests/validation/validation_function.h"
#include "software/test_util/test_util.h"
#include "software/time/duration.h"
#include "software/world/world.h"

class DefensePlayTest : public SimulatedPlayTestFixture
{
   protected:
    Field field = Field::createSSLDivisionBField();
};

TEST_F(DefensePlayTest, test_defense_play)
{
    BallState ball_state(Point(0.9, 2.85), Vector(0, 0));
    auto friendly_robots = TestUtil::createStationaryRobotStatesWithId(
        {Point(-4.5, 0), Point(-3, 1.5), Point(-3, 0.5), Point(-3, -0.5), Point(-3, -1.5),
         Point(-3, -3.0)});
    setFriendlyGoalie(0);
    auto enemy_robots = TestUtil::createStationaryRobotStatesWithId({
        field.enemyGoalCenter(),
        field.enemyDefenseArea().negXNegYCorner(),
        field.enemyDefenseArea().negXPosYCorner(),
        Point(1, 3),
        Point(-1, -0.25),
        Point(-2, -1.25),
    });
    setEnemyGoalie(0);
    setAIPlay(TYPENAME(DefensePlay));
    // We set the referee command to stop so that the robots do not kick/shoot during the
    // test
    setRefereeCommand(RefereeCommand::STOP, RefereeCommand::STOP);

    std::vector<ValidationFunction> terminating_validation_functions = {
        [](std::shared_ptr<World> world_ptr, ValidationCoroutine::push_type& yield) {
            // Wait for all robots to come to a halt
            robotHalt(world_ptr, yield);
            // Attacker in front of enemy with the ball
            Rectangle attacker_rect(Point(0, 2.85), Point(0.9, 2));
            robotInPolygon(1, attacker_rect, world_ptr, yield);

            // Two friendly robots in position to shadow enemy robots. One is on the enemy
            // with the ball and the other is on the next highest threat
            Rectangle robot_four_shadowing_rect(Point(0.5, 2.5), Point(1, 2));
            Rectangle robot_five_shadowing_rect(Point(-2, -0.75), Point(-1.5, -1.25));
            robotInPolygon(4, robot_four_shadowing_rect, world_ptr, yield);
            robotInPolygon(5, robot_five_shadowing_rect, world_ptr, yield);

            // Two friendly crease defenders should be close to the goalie
            Point goalie_position = world_ptr->friendlyTeam().goalie()->position();
            Rectangle left_crease_defender_rect(
                Point(goalie_position.x(), goalie_position.y() + 0.4),
                Point(goalie_position.x() + 0.3, goalie_position.y()));
            Rectangle right_crease_defender_rect(
                Point(goalie_position.x(), goalie_position.y()),
                Point(goalie_position.x() + 0.3, goalie_position.y() - 0.3));
            robotInPolygon(2, left_crease_defender_rect, world_ptr, yield);
            robotInPolygon(3, right_crease_defender_rect, world_ptr, yield);
        }};

    std::vector<ValidationFunction> non_terminating_validation_functions = {
        [](std::shared_ptr<World> world_ptr, ValidationCoroutine::push_type& yield) {}};

    runTest(field, ball_state, friendly_robots, enemy_robots,
            terminating_validation_functions, non_terminating_validation_functions,
            Duration::fromSeconds(10));
}

TEST_F(DefensePlayTest, test_defense_play_one_immediate_threat)
{
    BallState ball_state(Point(-1.2, 0), Vector(0, 0));
    auto friendly_robots = TestUtil::createStationaryRobotStatesWithId(
        {Point(-4.5, 0), Point(-3, 1.5), Point(-3, 0.5), Point(-3, -0.5), Point(-3, -1.5),
         Point(-3, -3.0)});
    setFriendlyGoalie(0);
    auto enemy_robots = TestUtil::createStationaryRobotStatesWithId({
        field.enemyGoalCenter(),
        field.enemyDefenseArea().negXNegYCorner(),
        field.enemyDefenseArea().negXPosYCorner(),
        field.enemyDefenseArea().posXNegYCorner(),
        field.enemyDefenseArea().posXPosYCorner(),
        Point(-1, 0),
    });
    setEnemyGoalie(0);
    setAIPlay(TYPENAME(DefensePlay));
    // We set the referee command to stop so that the robots do not kick/shoot during the
    // test
    setRefereeCommand(RefereeCommand::STOP, RefereeCommand::STOP);

    std::vector<ValidationFunction> terminating_validation_functions = {
        [](std::shared_ptr<World> world_ptr, ValidationCoroutine::push_type& yield) {
            // Wait for all robots to come to a halt
            robotHalt(world_ptr, yield);
            // Attacker in front of enemy with the ball
            Rectangle attacker_rect(Point(-2.5, 0.5), Point(-1, -0.5));
            robotInPolygon(1, attacker_rect, world_ptr, yield);

            // Two friendly robots swarming around the immediate threat
            Rectangle swarming_rect(Point(-2, 1), Point(0, -1));
            robotInPolygon(4, swarming_rect, world_ptr, yield);
            robotInPolygon(5, swarming_rect, world_ptr, yield);

            // Two friendly crease defenders should be close to the goalie
            Point goalie_position = world_ptr->friendlyTeam().goalie()->position();
            Rectangle left_crease_defender_rect(
                Point(goalie_position.x(), goalie_position.y() + 0.3),
                Point(goalie_position.x() + 0.3, goalie_position.y()));
            Rectangle right_crease_defender_rect(
                Point(goalie_position.x(), goalie_position.y()),
                Point(goalie_position.x() + 0.3, goalie_position.y() - 0.3));
            robotInPolygon(2, left_crease_defender_rect, world_ptr, yield);
            robotInPolygon(3, right_crease_defender_rect, world_ptr, yield);
        }};

    std::vector<ValidationFunction> non_terminating_validation_functions = {
        [](std::shared_ptr<World> world_ptr, ValidationCoroutine::push_type& yield) {
            ballInPlay(world_ptr, yield);
        }};

    runTest(field, ball_state, friendly_robots, enemy_robots,
            terminating_validation_functions, non_terminating_validation_functions,
            Duration::fromSeconds(10));
}

TEST_F(DefensePlayTest, test_defense_play_close_to_net)
{
    BallState ball_state(Point(-2.4, 1), Vector(0, 0));
    auto friendly_robots = TestUtil::createStationaryRobotStatesWithId(
        {Point(-4.5, 0), Point(-3, 1.5), Point(-3, 0.5), Point(-3, -0.5), Point(-3, -1.5),
         Point(-3, -3.0)});
    setFriendlyGoalie(0);
    auto enemy_robots = TestUtil::createStationaryRobotStatesWithId({
        field.enemyGoalCenter(),
        Point(-2.3, 1.05),
        Point(-3.5, 2),
        Point(-1.5, 0),
        Point(-2.3, -1),
        Point(-3.8, -2),
    });
    setEnemyGoalie(0);
    setAIPlay(TYPENAME(DefensePlay));
    // We set the referee command to stop so that the robots do not kick/shoot during the
    // test
    setRefereeCommand(RefereeCommand::STOP, RefereeCommand::STOP);

    std::vector<ValidationFunction> terminating_validation_functions = {
        [](std::shared_ptr<World> world_ptr, ValidationCoroutine::push_type& yield) {
            // Wait for all robots to come to a halt
            robotHalt(world_ptr, yield);
            // Attacker in front of enemy with the ball
            Rectangle attacker_rect(Point(-3.5, 1.2), Point(-2.3, 0.2));
            robotInPolygon(1, attacker_rect, world_ptr, yield);

            // The rectangle for the right crease defender and one of the shadowing robots
            // is shared to make the test less brittle since the AI may assign different
            // friendly robots to the to each tactic.

            // Two friendly crease defenders should be close to the goalie
            Point goalie_position = world_ptr->friendlyTeam().goalie()->position();
            Rectangle left_crease_defender_rect(
                Point(goalie_position.x(), goalie_position.y() + 0.45),
                Point(goalie_position.x() + 0.55, goalie_position.y()));
            Rectangle right_crease_defender_and_shadow_rect(
                Point(goalie_position.x(), goalie_position.y()),
                Point(goalie_position.x() + 1.0, goalie_position.y() - 0.3));
            robotInPolygon(2, left_crease_defender_rect, world_ptr, yield);
            robotInPolygon(3, right_crease_defender_and_shadow_rect, world_ptr, yield);

            // Two friendly robots in position to shadow enemy robots. One is on the enemy
            // with the ball and the other is on the next highest threat
            Rectangle robot_five_shadowing_rect(Point(-2.75, -0.5), Point(-2.25, -1));
            robotInPolygon(4, right_crease_defender_and_shadow_rect, world_ptr, yield);
            robotInPolygon(5, robot_five_shadowing_rect, world_ptr, yield);
        }};

    std::vector<ValidationFunction> non_terminating_validation_functions = {
        [](std::shared_ptr<World> world_ptr, ValidationCoroutine::push_type& yield) {
            ballInPlay(world_ptr, yield);
        }};

    runTest(field, ball_state, friendly_robots, enemy_robots,
            terminating_validation_functions, non_terminating_validation_functions,
            Duration::fromSeconds(10));
}

TEST(DefensePlayIsApplicableInvariantHoldsTest,
     test_invariant_and_is_applicable_enemy_possession)
{
    auto play_config = std::make_shared<ThunderbotsConfig>()->getPlayConfig();
    auto world       = ::TestUtil::createBlankTestingWorld();
    world.setTeamWithPossession(TeamSide::ENEMY);

    DefensePlay defense_play = DefensePlay(play_config);

    // Gamestate isPlaying
    world.updateGameState(
        ::TestUtil::createGameState(RefereeCommand::FORCE_START, RefereeCommand::HALT));

    EXPECT_TRUE(defense_play.isApplicable(world));
    EXPECT_TRUE(defense_play.invariantHolds(world));

    // Gamestate isHalted
    world.updateGameState(
        ::TestUtil::createGameState(RefereeCommand::HALT, RefereeCommand::FORCE_START));

    EXPECT_FALSE(defense_play.isApplicable(world));
    EXPECT_FALSE(defense_play.invariantHolds(world));
}

TEST(DefensePlayIsApplicableInvarantHoldsTest,
     test_invariant_and_is_applicable_friendly_possession)
{
    auto play_config = std::make_shared<ThunderbotsConfig>()->getPlayConfig();
    auto world       = ::TestUtil::createBlankTestingWorld();
    world.setTeamWithPossession(TeamSide::FRIENDLY);

    DefensePlay defense_play = DefensePlay(play_config);

    // Gamestate isPlaying
    world.updateGameState(
        ::TestUtil::createGameState(RefereeCommand::FORCE_START, RefereeCommand::HALT));

    EXPECT_FALSE(defense_play.isApplicable(world));
    EXPECT_FALSE(defense_play.invariantHolds(world));

    // Gamestate isHalted
    world.updateGameState(
        ::TestUtil::createGameState(RefereeCommand::HALT, RefereeCommand::FORCE_START));

    EXPECT_FALSE(defense_play.isApplicable(world));
    EXPECT_FALSE(defense_play.invariantHolds(world));
}
