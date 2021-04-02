# -*- coding: utf-8 -*-
"""
Ambrosia - Potato Boss Simulations

"""

# basics
import matplotlib.pyplot as plt
import numpy as np
import random
from enum import IntEnum

"""
Simulation Constants

"""
NUM_TURNS = 15

class Player(IntEnum):
    Raoul = 0
    Ember = 1
    Taji = 2
    Chia = 3

class Stat(IntEnum):
    HP = 0
    IsShielded = 1
    IsBuffed = 2
    IsDebuffed = 3
    IsStunned = 4
    IsCCImmune = 5

# player stats
MAXHP_PLAYER = np.zeros(4)
MAXHP_PLAYER[Player.Raoul] = 100
MAXHP_PLAYER[Player.Taji] = 70
MAXHP_PLAYER[Player.Ember] = 80
MAXHP_PLAYER[Player.Chia] = 80

# player skill damage
RAOUL_SKILL1 = 30
RAOUL_SKILL2 = 0
RAOUL_SKILL3 = 15
BUFF_STRENGTH = 0.4

TAJI_SKILL1 = 20
TAJI_SKILL2 = 20
TAJI_SKILL3 = 8

EMBER_SKILL1 = 25
EMBER_SKILL2 = 50
EMBER_SKILL3 = 40

CHIA_SKILL1 = 20
CHIA_HEAL = 30
CHIA_SKILL2 = 15
CHIA_SKILL3 = 0
SHIELD_STRENGTH = 30
DEBUFF_STRENGTH = 0.4

# enemy stats
MAXHP_POTATO = 400


"""
Simulation Utilities

"""
def get_target_indices(entities, numTargets, randomize):
    "returns the indices of requested alive targets"
    if numTargets > entities.size:
        numTargets = entities.size
    
    # initialize return targets
    targets = np.zeros(numTargets).astype(int)
    targets.fill(-1)
    
    # the indices to iterate through
    indices = np.arange(entities.shape[0])

    if randomize:
        np.random.shuffle(indices)

    counter = 0
    for i in indices:
        # add alive entities to the target
        if entities[i][Stat.HP] > 0:
            targets[counter] = i
            
        # stop once we've found enough targets
        counter += 1
        if counter >= numTargets:
            break
    
    return targets


def clamp_hp(entities):
    "ensures entity hp does not drop below zero"
    
    for entity in entities:
        entity[Stat.HP] = max(0, entity[Stat.HP])

def damage_player(players, i, damage):
    "calculates damage for players, including shield effects"
    if players[i][Stat.IsShielded] >= damage:
        # the shield absorbs all the damage
        players[i][Stat.IsShielded] -= damage 
    else:
        # the shield is depleted and the player takes damage
        damage -= max(0, players[i][Stat.IsShielded])
        players[i][Stat.IsShielded] = 0
        players[i][Stat.HP] = max(0,  players[i][Stat.HP] - damage)
            
            

"""
Player Simulators

"""
def simulate_raoul(skill, enemies, players):
    if enemies.size <= 0:
        return
    
    # calculate current strength
    strength = 1 + players[Player.Raoul][Stat.IsBuffed] * BUFF_STRENGTH
    
    if skill == 1:
        # single-target melee damage to "closest" enemy
        target = get_target_indices(enemies, 1, False)[0]
        if target >= 0:
            enemies[target][Stat.HP] -= RAOUL_SKILL1 * strength
    
    if skill == 2:
        players[Player.Raoul][Stat.IsBuffed] = 1
        isBuffed = random.randint(1, 10)
        
        # assume Ember is within range of his buff 90% of the time
        if isBuffed <= 9:
            players[Player.Ember][Stat.IsBuffed] = 1
        
        # assume Taji is within range of his buff 60% of the time
        if isBuffed <= 6:
            players[Player.Taji][Stat.IsBuffed] = 1
        
        # assume Chia is within range of his buff 40% of the time
        if isBuffed <= 4:
            players[Player.Chia][Stat.IsBuffed] = 1
        
    if skill == 3:
        # assume hits 1-3 random mobs
        rng = random.randint(1, 3)
        max_hit = min(rng, enemies.size)
        targets = get_target_indices(enemies, max_hit, True)
        for i in targets:
            if i >= 0:
                enemies[i, Stat.HP] -= RAOUL_SKILL3 * strength


def simulate_taji(skill, enemies, players):
    if enemies.size <= 0:
        return
    
    # calculate current strength
    strength = 1 + players[Player.Raoul][Stat.IsBuffed] * BUFF_STRENGTH
    
    if skill == 1:
        # assume hits 2-3 random mobs
        rng = random.randint(2, 3)
        max_hit = min(rng, enemies.size)
        targets = get_target_indices(enemies, max_hit, True)
        for i in targets:
            if i >= 0:
                enemies[i, Stat.HP] -= TAJI_SKILL1 * strength
    
    if skill == 2:
        # hit a random enemy
        target = get_target_indices(enemies, 1, True)[0]
        if target >= 0:
           enemies[target][Stat.HP] -= TAJI_SKILL2 * strength
        
        # stun the target if they aren't cc immune
        if (enemies[target, Stat.IsCCImmune] == 0):
            enemies[target, Stat.IsStunned] = 1
        
    if skill == 3:
        # deal damage to all enemies
        for enemy in enemies:
            enemy[Stat.HP] -= TAJI_SKILL3 * strength
        
        # heal all allies up to max HP
        for i, playerHP in enumerate(players[:,Stat.HP]):
            if players[i][Stat.HP] > 0:
                players[i][Stat.HP] = min(playerHP + TAJI_SKILL3 * strength, MAXHP_PLAYER[i])
      

def simulate_ember(skill, enemies, players):
    if enemies.size <= 0:
        return    
    
    # calculate current strength
    strength = 1 + players[Player.Raoul][Stat.IsBuffed] * BUFF_STRENGTH

    if skill == 1:
        # assume hits 2-3 random mobs
        rng = random.randint(2, 3)
        max_hit = min(rng, enemies.size)
        targets = get_target_indices(enemies, max_hit, True)
        for i in targets:
            if i >= 0:
                enemies[i, Stat.HP] -= EMBER_SKILL1 * strength
    
    if skill == 2:
        # hit the "closest" enemy
        target = get_target_indices(enemies, 1, False)[0]
        if target >= 0:
            enemies[target, Stat.HP] -= EMBER_SKILL2 * strength
        
    if skill == 3:
        # assume hits 2-3 random mobs
        rng = random.randint(2, 3)
        max_hit = min(rng, enemies.size)
        targets = get_target_indices(enemies, max_hit, True)
        for i in targets:
            if i >= 0:
                enemies[i, Stat.HP] -= EMBER_SKILL3 * strength
                

def simulate_chia(skill, enemies, players):
    if enemies.size <= 0:
        return    

    # calculate current strength
    strength = 1 + players[Player.Raoul][Stat.IsBuffed] * BUFF_STRENGTH
    
    if skill == 1:
        # heals either Taji, or Raoul + Ember
        # prioritize healing Raoul
        if (players[Player.Raoul][Stat.HP] < MAXHP_PLAYER[Player.Raoul] / 2):
            if players[Player.Raoul][Stat.HP] > 0:
                players[Player.Raoul][Stat.HP] = min(players[Player.Raoul][Stat.HP] 
                                      + CHIA_HEAL * strength, 
                                      MAXHP_PLAYER[Player.Raoul])
            if players[Player.Ember][Stat.HP] > 0:
                players[Player.Ember][Stat.HP] = min(players[Player.Ember][Stat.HP] 
                                      + CHIA_HEAL * strength, 
                                      MAXHP_PLAYER[Player.Ember])
                
            # if healing a melee player, assume it hits an enemy too
            # hit the "closest" enemy
            target = get_target_indices(enemies, 1, False)[0]
            if target >= 0:
               enemies[target, Stat.HP] -= CHIA_SKILL1 * strength
                
        # heal Taji if Raoul's a-okay
        else:
            if players[Player.Taji][Stat.HP] > 0:
                players[Player.Taji][Stat.HP] = min(players[Player.Taji][Stat.HP] 
                                      + CHIA_HEAL * strength, 
                                      MAXHP_PLAYER[Player.Taji])
    
   
    if skill == 2:
         # hit and debuff a random enemy
         target = get_target_indices(enemies, 1, True)[0]
         if target >= 0:
             enemies[target, Stat.HP] -= CHIA_SKILL2 * strength
             enemies[target, Stat.IsDebuffed] = 1
        
    if skill == 3:
        # shield all allies
        for i in range(4):
            players[i][Stat.IsShielded] = SHIELD_STRENGTH


"""
Enemy Simulators

"""
def simulate_potato(skill, enemies, players):
    if enemies.size <= 0:
        return    

    # calculate current strength
    strength = 1 - enemies[0, Stat.IsDebuffed] * DEBUFF_STRENGTH
    
    if skill == 1:
        # hit closest player
        target = get_target_indices(enemies, 1, False)[0]
        damage = 40 * strength
        damage_player(players, target, damage)
    
    if skill == 2:
         # ultimate
         damage = 70
         for i in range(4):
            damage_player(players, i, damage)


"""
Full Simulations

"""

def simulate_random():
    # number of turns to stimulate
    turns = np.arange(0, NUM_TURNS)
    
    # initialize player matrix
    # each row is a player, each column is a stat
    players = np.zeros((4, 6))
    
    # initialize each player to have max HP
    for i in range(4):
        players[i][Stat.HP] = MAXHP_PLAYER[i]
    
    # enemies
    enemies = np.zeros((1,6)) # just a potato for now
    enemies[0][Stat.HP] = MAXHP_POTATO
    enemies[0][Stat.IsCCImmune] = 1
    ultCounter = 2
    
    
    players_hp = np.zeros((4, NUM_TURNS))
    enemies_hp = np.zeros((1, NUM_TURNS))
    
    # keep track of which skills were used
    skills_used = np.zeros((5, NUM_TURNS))
    result = np.array([0, NUM_TURNS])
    
    # begin simulation - random actions
    for turn in turns:      
        # log HP at the start of the turn
        for i, playerHP in enumerate(players_hp[:,turn]):
            players_hp[i][turn] = players[i][Stat.HP]
        
        for i, enemyHP in enumerate(enemies_hp[:,turn]):
            enemies_hp[i][turn] = enemies[i][Stat.HP]
        
        # players take their turns
        if (players[Player.Raoul][Stat.HP] > 0):
            skill = random.randint(1, 3)
            simulate_raoul(skill, enemies, players)
            skills_used[Player.Raoul][turn] = skill
        
        if (players[Player.Taji][Stat.HP] > 0):
            skill = random.randint(1, 3)
            simulate_taji(skill, enemies, players)
            skills_used[Player.Taji][turn] = skill
        
        if (players[Player.Ember][Stat.HP] > 0):
            skill = random.randint(1, 3)
            simulate_ember(skill, enemies, players)
            skills_used[Player.Ember][turn] = skill
        
        if (players[Player.Chia][Stat.HP] > 0):
            skill = random.randint(1, 3)
            simulate_chia(skill, enemies, players)
            skills_used[Player.Chia][turn] = skill
        
        # check if the game is over
        if enemies[0, Stat.HP] <= 0:
            # print(f"Victory on turn {turn}")
            for i in range(turn, NUM_TURNS):
                for p in range(4):
                    players_hp[p][i] = players[p][Stat.HP]
            result = np.array([1, turn])
            break;
        
        # enemies take their turns
        # just the potato for now
        if enemies[0, Stat.HP] > 0:
            if ultCounter > 0 and (turn == 0 or enemies[0, Stat.HP] < (MAXHP_POTATO / 2)):
                # use ultimate on first turn or when first time below 50% HP
                simulate_potato(2, enemies, players)
                skills_used[4][turn] = 2
                ultCounter -= 1
                
            else:
                simulate_potato(1, enemies, players)
                skills_used[4][turn] = 1
    
        
        # clear all buffs
        for i in range(4):
            players[i, Stat.IsBuffed] = 0
            players[i, Stat.IsShielded] = 0
        
        enemies[0, Stat.IsDebuffed] = 0
        
        if np.sum(players[:, Stat.HP]) <= 0:
            # print(f"Defeat on turn {turn}")
            for i in range(turn, NUM_TURNS):
                enemies_hp[0][i] = enemies[0][Stat.HP]
                
            result = np.array([0, turn])
            break;
        
    # fig, hpplot = plt.subplots()
    # # plot the simulation results
    # hpplot.plot(turns, players_hp[Player.Raoul], label="Raoul")
    # hpplot.plot(turns, players_hp[Player.Ember], label="Ember")
    # hpplot.plot(turns, players_hp[Player.Taji], label="Taji")
    # hpplot.plot(turns, players_hp[Player.Chia], label="Chia")
    
    # # just the potato for now
    # hpplot.plot(turns, enemies_hp[0], label="Potato")
    
    # hpplot.set_xlabel("Turn")
    # hpplot.set_ylabel("HP")
    # plt.legend()
    
    
    # # print the skills that were used
    # print(skills_used)
    
    # skillplot = hpplot.twinx()
    # skillplot.set_ylabel('Skill')
    # skillplot.scatter(turns, skills_used[Player.Raoul], label="Raoul")
    # skillplot.scatter(turns, skills_used[Player.Ember], label="Ember")
    # skillplot.scatter(turns, skills_used[Player.Taji], label="Taji")
    # skillplot.scatter(turns, skills_used[Player.Chia], label="Chia")
    # skillplot.scatter(turns, skills_used[4], label="Potato")
    # skillplot.set_ylim (0.5, 3.5)
    
    # fig.tight_layout()
    # plt.show()
    return result;
    
    
def simulate_strategic():
    # number of turns to stimulate
    turns = np.arange(0, NUM_TURNS)
    
    # initialize player matrix
    # each row is a player, each column is a stat
    players = np.zeros((4, 6))
    
    # initialize each player to have max HP
    for i in range(4):
        players[i][Stat.HP] = MAXHP_PLAYER[i]
    
    # enemies
    enemies = np.zeros((1,6)) # just a potato for now
    enemies[0][Stat.HP] = MAXHP_POTATO
    enemies[0][Stat.IsCCImmune] = 1
    ultCounter = 2
    
    
    players_hp = np.zeros((4, NUM_TURNS))
    enemies_hp = np.zeros((1, NUM_TURNS))
    
    # keep track of which skills were used
    skills_used = np.zeros((5, NUM_TURNS))
    result = np.array([0, NUM_TURNS])
    
    # begin simulation - random actions
    for turn in turns:      
        # log HP at the start of the turn
        for i, playerHP in enumerate(players_hp[:,turn]):
            players_hp[i][turn] = players[i][Stat.HP]
        
        for i, enemyHP in enumerate(enemies_hp[:,turn]):
            enemies_hp[i][turn] = enemies[i][Stat.HP]
        
        # players take their turns
        if (players[Player.Raoul][Stat.HP] > 0):
            skill = random.randint(1, 3)
            simulate_raoul(skill, enemies, players)
            skills_used[Player.Raoul][turn] = skill
        
        if (players[Player.Taji][Stat.HP] > 0):
            skill = random.randint(1, 3)
            simulate_taji(skill, enemies, players)
            skills_used[Player.Taji][turn] = skill
        
        if (players[Player.Ember][Stat.HP] > 0):
            skill = random.randint(1, 3)
            simulate_ember(skill, enemies, players)
            skills_used[Player.Ember][turn] = skill
        
        if (players[Player.Chia][Stat.HP] > 0):
            skill = random.randint(1, 3)
            if turn == 0:
                skill = 3
            if (enemies[0][Stat.HP] < 200 and ultCounter > 0):
                skill = 3
            
            simulate_chia(skill, enemies, players)
            skills_used[Player.Chia][turn] = skill
        
        
        # check if the game is over
        if enemies[0, Stat.HP] <= 0:
            # print(f"Victory on turn {turn}")
            for i in range(turn, NUM_TURNS):
                for p in range(4):
                    players_hp[p][i] = players[p][Stat.HP]
            result = np.array([1, turn])
            break
        
        # enemies take their turns
        # just the potato for now
        if enemies[0, Stat.HP] > 0:
            if ultCounter > 0 and (turn == 0 or enemies[0, Stat.HP] < (MAXHP_POTATO / 2)):
                # use ultimate on first turn or when first time below 50% HP
                simulate_potato(2, enemies, players)
                skills_used[4][turn] = 2
                ultCounter -= 1
                
            else:
                simulate_potato(1, enemies, players)
                skills_used[4][turn] = 1
        
        # clear all buffs
        for i in range(4):
            players[i, Stat.IsBuffed] = 0
            players[i, Stat.IsShielded] = 0
        
        enemies[0, Stat.IsDebuffed] = 0
        
        if np.sum(players[:, Stat.HP]) <= 0:
            # print(f"Defeat on turn {turn}")
            for i in range(turn, NUM_TURNS):
                enemies_hp[0][i] = enemies[0][Stat.HP]
                
            result = np.array([0, turn])
            break;
    
    # fig, hpplot = plt.subplots()
    # # plot the simulation results
    # hpplot.plot(turns, players_hp[Player.Raoul], label="Raoul")
    # hpplot.plot(turns, players_hp[Player.Ember], label="Ember")
    # hpplot.plot(turns, players_hp[Player.Taji], label="Taji")
    # hpplot.plot(turns, players_hp[Player.Chia], label="Chia")
    
    # # just the potato for now
    # hpplot.plot(turns, enemies_hp[0], label="Potato")
    
    # hpplot.set_xlabel("Turn")
    # hpplot.set_ylabel("HP")
    # plt.legend()
    
    
    # # print the skills that were used
    # print(skills_used)
    
    # skillplot = hpplot.twinx()
    # skillplot.set_ylabel('Skill')
    # skillplot.scatter(turns, skills_used[Player.Raoul], label="Raoul")
    # skillplot.scatter(turns, skills_used[Player.Ember], label="Ember")
    # skillplot.scatter(turns, skills_used[Player.Taji], label="Taji")
    # skillplot.scatter(turns, skills_used[Player.Chia], label="Chia")
    # skillplot.scatter(turns, skills_used[4], label="Potato")
    # skillplot.set_ylim (0.5, 3.5)
    
    # fig.tight_layout()
    # plt.show()
    
    return result

def simulate_super_strategic():
    # number of turns to stimulate
    turns = np.arange(0, NUM_TURNS)
    
    # initialize player matrix
    # each row is a player, each column is a stat
    players = np.zeros((4, 6))
    
    # initialize each player to have max HP
    for i in range(4):
        players[i][Stat.HP] = MAXHP_PLAYER[i]
    
    # enemies
    enemies = np.zeros((1,6)) # just a potato for now
    enemies[0][Stat.HP] = MAXHP_POTATO
    enemies[0][Stat.IsCCImmune] = 1
    ultCounter = 2
    
    
    players_hp = np.zeros((4, NUM_TURNS))
    enemies_hp = np.zeros((1, NUM_TURNS))
    
    # keep track of which skills were used
    skills_used = np.zeros((5, NUM_TURNS))
    result = np.array([0, NUM_TURNS])
    
    # begin simulation - random actions
    for turn in turns:      
        # log HP at the start of the turn
        for i, playerHP in enumerate(players_hp[:,turn]):
            players_hp[i][turn] = players[i][Stat.HP]
        
        for i, enemyHP in enumerate(enemies_hp[:,turn]):
            enemies_hp[i][turn] = enemies[i][Stat.HP]
        
        # players take their turns
        if (players[Player.Raoul][Stat.HP] > 0):
            skill = random.randint(1, 3)
            # reduce damage before ult is over
            # Raoul's 2nd skill buffs heals too, so it's good
            if (ultCounter > 0):
                skill = random.randint(2, 3)
            simulate_raoul(skill, enemies, players)
            skills_used[Player.Raoul][turn] = skill
        
        if (players[Player.Taji][Stat.HP] > 0):
            skill = random.randint(1, 3)
            
            # heal up as much as possible before ult is over
            if (ultCounter > 0):
                skill = 3
            simulate_taji(skill, enemies, players)
            skills_used[Player.Taji][turn] = skill
        
        if (players[Player.Ember][Stat.HP] > 0):
            skill = random.randint(1, 3)
            
            # reduce damage before ult is over
            if (ultCounter > 0):
                skill = 1
            else: # spam damage
                skill = 2
            simulate_ember(skill, enemies, players)
            skills_used[Player.Ember][turn] = skill
        
        if (players[Player.Chia][Stat.HP] > 0):
            skill = 1 # heal as much as possible
            if turn == 0:
                skill = 3
            if (enemies[0][Stat.HP] < 200 and ultCounter > 0):
                skill = 3
            
            simulate_chia(skill, enemies, players)
            skills_used[Player.Chia][turn] = skill
        
        # check if the game is over
        if enemies[0, Stat.HP] <= 0:
            # print(f"Victory on turn {turn}")
            for i in range(turn, NUM_TURNS):
                for p in range(4):
                    players_hp[p][i] = players[p][Stat.HP]
            result = np.array([1, turn])
            break
        
        # enemies take their turns
        # just the potato for now
        if enemies[0, Stat.HP] > 0:
            if ultCounter > 0 and (turn == 0 or enemies[0, Stat.HP] < (MAXHP_POTATO / 2)):
                # use ultimate on first turn or when first time below 50% HP
                simulate_potato(2, enemies, players)
                skills_used[4][turn] = 2
                ultCounter -= 1
                
            else:
                simulate_potato(1, enemies, players)
                skills_used[4][turn] = 1
        
        # clear all buffs
        for i in range(4):
            players[i, Stat.IsBuffed] = 0
            players[i, Stat.IsShielded] = 0
        
        enemies[0, Stat.IsDebuffed] = 0
        
        if np.sum(players[:, Stat.HP]) <= 0:
            # print(f"Defeat on turn {turn}")
            for i in range(turn, NUM_TURNS):
                enemies_hp[0][i] = enemies[0][Stat.HP]
                
            result = np.array([0, turn])
            break;
    
    fig, hpplot = plt.subplots()
    # plot the simulation results
    hpplot.plot(turns, players_hp[Player.Raoul], label="Raoul")
    hpplot.plot(turns, players_hp[Player.Ember], label="Ember")
    hpplot.plot(turns, players_hp[Player.Taji], label="Taji")
    hpplot.plot(turns, players_hp[Player.Chia], label="Chia")
    
    # just the potato for now
    hpplot.plot(turns, enemies_hp[0], label="Potato")
    
    hpplot.set_xlabel("Turn")
    hpplot.set_ylabel("HP")
    plt.legend()
    
    
    # print the skills that were used
    print(skills_used)
    
    skillplot = hpplot.twinx()
    skillplot.set_ylabel('Skill')
    skillplot.scatter(turns, skills_used[Player.Raoul], label="Raoul")
    skillplot.scatter(turns, skills_used[Player.Ember], label="Ember")
    skillplot.scatter(turns, skills_used[Player.Taji], label="Taji")
    skillplot.scatter(turns, skills_used[Player.Chia], label="Chia")
    skillplot.scatter(turns, skills_used[4], label="Potato")
    skillplot.set_ylim (0.5, 3.5)
    
    fig.tight_layout()
    plt.show()
        
    return result
    
    
"""
Main

"""
if __name__ == '__main__':
    
    num_test = 1000
    test_num = np.arange(num_test)
    results = np.zeros((num_test, 2))
    for i in range(num_test):
        results[i] = simulate_random()
    
    # plot the simulation results
    plt.scatter(test_num, results[:, 1], s=100, c=results[:, 0])

    plt.xlabel("Test")
    plt.ylabel("Turns Taken")
    plt.yticks(np.arange(NUM_TURNS))
    
    # calculate percentage of wins
    percentage = np.count_nonzero(results[:, 0]) / num_test
    print(f"Victory Rate: {percentage * 100}%")

