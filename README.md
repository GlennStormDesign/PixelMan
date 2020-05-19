# Pixel Man

[image goes here]
[PixelMan](http://hotironproductions.com/pixelman/image/pmcpp.png?raw=true "PixelMan")

C++ SFML 2D Zombie Apocalypse Shooter in an old-school arcade style

## Try it
[Download .zip Windows Game](http://hotironproductions.com/pixelman/doc/Pixel%20Man%20Game.zip)

## What Up?
I've been making games with code for over three decades, and since 2009, working with C# (in unity). So, while learning C++ (and before that C) has been challenging in the past, since Sept 2019, I've been able to get past major hurdles and get down to it.

Your character will fend off hoards of zombies with items he finds on the street. To play, use the arrow keys (or WASD) to move, and either CTRL key to shoot. Run over pickup weapons as they will spawn in one of two locations. You lose when health is zero, you win if you kill 'enough' zombies for their infection to completely dissolve them. (ew!)

The project was meant as a means to try SFML, and implement graphics, animation and sound with time, a viewport, etc. The emphasis was not as concerned with optimization, so it does use fairly arbitrary art and audio file sizes, standard variable memory allocations, and makes up to 32 draw calls per frame. (not great, but at least zombie hoard draw sorting was cleverly mitigated)

Thank you to unknown composers and artists who supplied the art and sound for this game, which I freely admit stealing for this game programming exercise. (thank you, don't sue)

## Primary Features
* Music and SFX
* Viewport and HUD
* Graphics and Animation
* Weapons and Projectiles
* Combat and Health Management
