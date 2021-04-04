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
static const char* AchievementText[] = { "Finished the tutorial!", "Beat a level without anyone dying!", "Beat a recipe!", 
										 "Beat a recipe without anyone dying", "Defeated a boss with low HP" };


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
	EventListenerInfo beatLevelListener;
	EventListenerInfo defeatedBossListener;

	void onFinishedTutorialEvent();
	void onReachedBossEvent();
	void onBeatLevelEvent();
	void onDefeatedBossEvent();

	bool isTracking(Achievement item);
};