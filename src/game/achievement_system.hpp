#include <vector>
#include <string>
#include "entities/tiny_ecs.hpp"
#include "event_system.hpp"
#include "events.hpp"


enum Achievement {
	FINISH_TUTORIAL,
	BEAT_LEVEL_ALL_ALIVE,
	BEAT_RECIPE,
	BEAT_RECIPE_ALL_ALIVE,
	BEAT_BOSS_LOW_HP,
	SIZE_OF_ENUM
};
static const char* AchievementText[] = { "Finished the tutorial!", "Beat the level without anyone dying!", "Beat the recipe!", 
										 "Beat the recipe without anyone dying", "Defeated the boss with low HPs" };


class AchievementSystem {
protected:
	std::list<Achievement> achievements;
	std::list<Achievement> tracking;

	AchievementSystem();
	~AchievementSystem();

public:
	// Returns the singleton instance of this system
	static AchievementSystem& instance() {
		static AchievementSystem achievementSystem;
		return achievementSystem;
	}

	void addAchievement(Achievement item) { achievements.push_back(item); };
	std::list<Achievement> getAchievements() { return achievements; };
	void addToTracking(Achievement item);
	void removeFromTracking(Achievement item);

private:
	EventListenerInfo finishedTutorialListener;
	EventListenerInfo reachedBossListener;
	EventListenerInfo defeatedBossListener;

	void onFinishedTutorialEvent();
	void onReachedBossEvent();
	void onDefeatedBossEvent(const DefeatedBossEvent& event);

	bool isTracking(Achievement item);
};