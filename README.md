# PICO-8 Game Jam Quest


## Game Overview
**Game Designer:** [Your Name]\
**Game Title:** [Insert Game Title]\
**GitLab Repository Link:** [Insert Link]

### Elevator Pitch
Provide a concise 1-3 sentence summary of your game, highlighting its core concept and what makes it unique.

I created a maze navigation game that displays a couple technical skills: 1st, mazes can be completely randomly generated using different seeds via Kruskal's algorithm. You can modify the code to create a different maze each run of the game, although, for convenience, I have hard coded the random seed to make it more convenient to debug. 2nd, there are 3 powerups you can toggle through: wall destroy, teleportation, and slowdown time. 3rd, I actually simulate a 3D environment, similar to DOOM, using the DDA algorithm for ray marching and finding intersections along a raycast. See the raycast tab for implementation details of the DDA algorithm.

### Theme Interpretation
Explain how your game aligns with the given theme. Describe your creative interpretation and how it is reflected through gameplay mechanics, narrative, or visuals.

The player is trapped in the maze, and must reach the end (to which there is none). Since mazes can be randomly generated, the levels progressively get harder naturally. Although there is less emphasis on narrative, I think this game's aesthetic strives more towards challenge and abnegation, for the player to challenge themselves to find the end of a maze. I also think making the game 3D and adding powerups makes the nature of the game more interactive than a simple 2D maze game.

### Controls
List the controls for your game (e.g., arrow keys for movement, Z for action).
Mouse controls are an option. Arrow keys move the player similar to a 3D game. Z toggles abilities via a scroll wheel feature, and X performs the ability.

## MDA Framework

### Aesthetics
Identify the **primary** aesthetic(s) of your game and explain how the elements contribute to it. Consider aspects such as visuals, sound, and gameplay elements that support the intended player experience.

I think the primary aesthetic is challenge. Mazes naturally become harder to progress, and the texturing of the mazes in my game naturally makes it harder to differentiate between where you've been and where your destination is. Use of gameplay abilities to achieve a best run through the maze also makes the game more interactive, and the music allows can keep the player engaged even in the absence of any movement or abilties. 

### Dynamics
Outline the core interactions and player experiences that emerge from the mechanics. How does the game encourage specific behaviors, strategies, or engagement over time?

I think one core mechanic is the use of the abilities destroy wall and teleport. These are interesting abilities because you might want to save a checkpoint somewhere to go back to it as you progress through the maze and hit a dead end, but you need to be careful where you place your marker for teleportation. Furthermore, destroy wall charges are limited, so you might need to consider the best wall to destroy to benefit your speed run through the maze.

### Mechanics
Detail the core mechanics that define your game, such as movement, player actions, obstacles, and unique features. Explain how these mechanics work together to create an engaging gameplay experience.

Players move through the maze using WASD and collide with walls via collision detection. Players goal is to reach the endpoint as fast as possible so that they can continue playing the game as it gets harder and harder. Players have access to three abilities that will help them achieve a successful run in the time alloted, and can use all three abilities at the same time to engage with the map in an intelligent way. 

## External Resources

### Assets
List any external assets used (e.g., sprite graphics, sound effects, music) and their sources. Provide proper attribution.

None, assets are all mine.

### Code
List any external code used, including tutorials or example projects. Provide links and proper citations.

I had to search up algorithms for Kruskal's, DDA, and BFS. Some useful links I used: 
https://www.jamisbuck.org/presentations/rubyconf2011/index.html#thanks
https://github.com/marukrap/RoguelikeDevResources?tab=readme-ov-file

## Code Documentation
You do not need to modify the Code Documentation section of the readme. This seciton serves as a reminder to make sure that your in-code documentation is clear and informative. Important sections such as function or files should be accompanied by comments describing their purpose and functionality.  

Example:  
```lua
-- Handles sprite movement based on arrow key input
function move_sprite()
  if btn(0) then player.x -= 1 end  -- Move left
  if btn(1) then player.x += 1 end  -- Move right
end