#include <SFML/Graphics.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Audio.hpp>

// PIXEL MAN
// Zombie shooter by Glenn Storm

bool splash() {
    sf::RenderWindow window(sf::VideoMode(320, 180), "splash", sf::Style::None);
    sf::VideoMode vm(sf::VideoMode::getDesktopMode());
    window.setPosition( sf::Vector2i( (( vm.width / 2 ) - 160) , (( vm.height / 2 ) - 90) ) );

    // ensure splash is displayed
    sf::Clock safetyClock;
    safetyClock.restart();

    sf::Font f;
    bool loadOkay = true;
    if ( !f.loadFromFile("image/arcadeClassic.ttf") ) {
        // error
        loadOkay = false;
    }

    sf::Text t;
    sf::Text subT;
    t.setFont(f);
    sf::Vector2f v = sf::Vector2f();
    v.x = 105;
    v.y = 70;
    t.setPosition(v);
    t.setCharacterSize(24);
    t.setFillColor(sf::Color::White);
    t.setString("Pixel Man");
    subT = t;
    v.y = 92;
    subT.setPosition(v);
    subT.setCharacterSize(12);
    subT.setString("GHS Game  Jan 2020");

    sf::SoundBuffer sb;
    if ( !sb.loadFromFile("audio/musicStingStart.wav") ) {
        // error
        loadOkay = false;
    }
    sf::Sound s;
    s.setBuffer(sb);
    s.play();

    while (window.isOpen() && ( s.getStatus() == s.Playing || safetyClock.getElapsedTime().asSeconds() < 3.5f ) )
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(t);
        window.draw(subT);
        window.display();
    }

    return loadOkay;
}

struct Shot {
    sf::CircleShape proj;
    sf::Vector2f fireVector;
    sf::Clock fireLife;
};

struct Zombie {
    sf::Sprite zS;
    bool inPlay;
    bool facingLeft;
    bool attacking;
    bool dying;
    int health;
    int animFrame;
    sf::Clock animTimer;
    sf::Clock attackTimer;
};

int main()
{
    // rand seed
    srand(time(NULL));

    // splash screen
    if ( !splash() ) {
        // handle no image or audio access
        return -1;
    }

    // main game
    sf::RenderWindow rWin( sf::VideoMode( 800, 450, 256 ), "PIXEL MAN" );
    sf::VideoMode vm(sf::VideoMode::getDesktopMode());
    rWin.setPosition( sf::Vector2i( (( vm.width / 2 ) - 400) , (( vm.height / 2 ) - 225) ) );

    // MUSIC
    sf::SoundBuffer msba;
    sf::SoundBuffer msbb;
    sf::SoundBuffer msbc;
    bool soundOkay = true;
    if ( !msba.loadFromFile("audio/musicLoopA.wav") ) {
        soundOkay = false;
    }
    if ( !msbb.loadFromFile("audio/musicLoopB.wav") ) {
        soundOkay = false;
    }
    if ( !msbc.loadFromFile("audio/musicLoopC.wav") ) {
        soundOkay = false;
    }

    sf::Sound ms;
    ms.setBuffer(msba);
    ms.setVolume(100.f);
    // will use ms for start chime, then music loops
    int musicCount = 0;

    // SFX
    sf::SoundBuffer sbShot, sbClick, sbClickBack, sbImpact, sbPickup;
    if ( !sbShot.loadFromFile("audio/sfxShoot.wav") || !sbClick.loadFromFile("audio/sfxClick.wav") || !sbClickBack.loadFromFile("audio/sfxClickBack.wav") || !sbImpact.loadFromFile("audio/sfxImpact.wav") || !sbPickup.loadFromFile("audio/sfxPowerup.wav") ) {
        // error
        soundOkay = false;
    }
    sf::Sound sShot;
    sf::Sound sClick;
    sf::Sound sClickBack;
    sf::Sound sImpact;
    sf::Sound sPickup;
    sShot.setBuffer(sbShot);
    sClick.setBuffer(sbClick);
    sClickBack.setBuffer(sbClickBack);
    sImpact.setBuffer(sbImpact);
    sPickup.setBuffer(sbPickup);

    sf::Texture bgT;
    if ( !bgT.loadFromFile("image/texBG.png", sf::IntRect(0, 0, 700, 350)) ) {
        // problem with image
    }
    sf::Sprite bgS;
    bgS.setTexture(bgT);
    sf::View mnVw;
    mnVw.setViewport(sf::FloatRect(0.f,-0.15f,2.f,4.f));
    float viewMoveRate = 36.f; //4.f; // * (1.f/125.f);

    sf::Texture plT;
    if ( !plT.loadFromFile("image/texPixelMan.png", sf::IntRect(127, 63, 1080, 720)) ) {
        // problem with image
    }
    // set sprite to frame within sprite sheet
    sf::Sprite plS;
    plS.setTexture(plT);
    plS.setTextureRect( sf::IntRect(720, 0, 360, 360) );
    plS.scale(sf::Vector2f(0.1f, 0.1f));
    plS.setPosition(sf::Vector2f(150.f, 220.f));
    plS.setOrigin(180.f,360.f);
    int playerWeapon = 0;
    int playerAnimOffset = 360; // flip to 360 for alt weapon anim frames
    bool playerHurt = false;
    bool playerScored = false;

    sf::Clock clk;
    sf::Time animTm;
    float frameRate = 0.15f;
    int frameNum = 0;
    float moveRate = 50.f; // * (1.f/90.f); // roughly per pixel
    bool vMoving, hMoving = false;
    bool faceLeft = false;

    sf::Texture pickT;
    if ( !pickT.loadFromFile("image/texWeapons.png", sf::IntRect(0,0,512,256)) ) {
        // problem with image
    }
    sf::Sprite puS;
    puS.setTexture(pickT);
    puS.setTextureRect( sf::IntRect(256,0,256,256) );
    puS.scale( sf::Vector2f(0.1f, 0.1f) );
    // set weapon pickup position off screen
    puS.setPosition(sf::Vector2f(580.f, 250.f));
    puS.setOrigin(128.f,256.f);
    int pickupType = 1;
    bool pickupLive = true;
    sf::Clock pickupTimer;

    bool playerFiring = false;
    bool playerFireReady = true;
    Shot* shots = new Shot[8];
    for ( int i=0; i<8; i++ ) {
        shots[i].proj.setRadius(2.f);
        shots[i].proj.setOutlineThickness(1.f);
        shots[i].proj.setFillColor(sf::Color::White);
        shots[i].proj.setOutlineColor(sf::Color::Yellow);
        shots[i].proj.setPosition(sf::Vector2f(-10.f,-10.f));
    }
    int shotIndex = 0;
    int shotMax = 8;
    float fireSpeed = 250.f; // *( 1/90.f );
    bool fireMoveLeft = false;
    // adjust for weapon type
    float shotLifeTime = 1.f;
    float reFireRate = 0.5f;
    sf::Clock fireClock;
    int ammoLeft = 12;

    int score = 0;
    int health = 100;
    sf::Font hudFont;
    hudFont.loadFromFile("image/arcadeClassic.ttf");
    sf::Text hudAmmo;
    sf::Text hudScore;
    sf::Text hudHealth;
    hudAmmo.setFont( hudFont );
    hudAmmo.setCharacterSize(20);
    hudAmmo.setFillColor(sf::Color::White);
    hudAmmo.setString("AMMO x11");
    hudScore.setFont( hudFont );
    hudScore.setCharacterSize(20);
    hudScore.setFillColor(sf::Color::White);
    hudScore.setString("SCORE 0");
    hudHealth.setFont( hudFont );
    hudHealth.setCharacterSize(20);
    hudHealth.setFillColor(sf::Color::White);
    hudHealth.setString("HEALTH 100");
    hudAmmo.setPosition( 20.f, 40.f );
    hudScore.setPosition( 190.f, 40.f );
    hudHealth.setPosition( 380.f, 40.f );
    sf::Clock hudTimer;

    sf::Texture zTex;
    zTex.loadFromFile("image/texPixelZombie.png");
    Zombie* hoard = new Zombie[20];
    int zCount = 0;
    float hoardMoveRate = 12.f;
    for ( int i=0; i<20; i++ ) {
        if ( ( rand() % 20 ) < 3 ) {
            hoard[i].inPlay = true;
            zCount++;
        }
        else
            hoard[i].inPlay = false;
        hoard[i].facingLeft = false;
        hoard[i].zS.setPosition(sf::Vector2f(30.f, 185.f));
        hoard[i].health = 4;
        hoard[i].dying = false;
        hoard[i].attacking = false;
        hoard[i].zS.setTexture(zTex);
        hoard[i].zS.setTextureRect( sf::IntRect(0,8,132,132) );
        hoard[i].zS.scale(sf::Vector2f(0.25f, 0.25f));
        // x = 30 or 670 (+/- 10) y = 185-275
        if ( rand() % 2 == 1 ) {
            hoard[i].zS.move(640.f, 0.f);
            hoard[i].facingLeft = true;
        }
        hoard[i].zS.move( (rand()%30)-15.f, (rand()%8)-4.f+(i*4.f)-4.f ); // pre-sorted zombie layers
        hoard[i].zS.setOrigin(66.f,132.f);
    }
    if ( zCount == 0 ) {
        // at least one
        hoard[10].inPlay = true;
    }

    bool gameRunning = false;
    bool gameStarting = true;
    bool gameEnding = false;
    bool playerWon = false;
    sf::SoundBuffer sbStart, sbLose, sbWin;
    sbStart.loadFromFile("audio/sfxBing.wav");
    sbLose.loadFromFile("audio/sfxChime.wav");
    sbWin.loadFromFile("audio/musicStingWin.wav");
    ms.setBuffer( sbStart );
    ms.play();

    sf::Text endText;
    endText.setFont(hudFont);
    endText.setCharacterSize(20);
    endText.setFillColor(sf::Color::White);
    endText.setString("GAME OVER");
    endText.setPosition( 190, 120 );

    int scoreToWin = 500 + (rand()%500);

    sf::Clock frameTimer;
    frameTimer.restart();
    float timeDelta = 0.f;

    rWin.setActive();
    while ( rWin.isOpen() )
    {
        // time delta
        timeDelta = frameTimer.restart().asSeconds();

        sf::Event event;
        while (rWin.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                rWin.close();
            if (event.type == sf::Event::Resized) {
                rWin.setSize( sf::Vector2u(800,480) );
                rWin.setPosition( sf::Vector2i( (( vm.width / 2 ) - 400) , (( vm.height / 2 ) - 225) ) );
            }
        }

        // manage music
        if ( gameStarting ) {
            if ( ms.getStatus() != sf::Sound::Playing ) {
                gameStarting = false;
                gameRunning = true;
                ms.setBuffer(msba);
                ms.setVolume(38.1f);
                ms.play();
            }
        }
        else if ( !gameRunning ) {
            float mVol = ms.getVolume();
            if ( mVol > 0.f && !gameEnding ) {
                mVol--;
                if ( mVol < 0.f ) {
                    mVol = 0.f;
                    ms.stop();
                }
                ms.setVolume(mVol);
                if ( mVol == 0.f ) {
                    if ( playerWon )
                        ms.setBuffer(sbWin);
                    else
                        ms.setBuffer(sbLose);
                    ms.setVolume(100.f);
                    ms.play();
                    gameEnding = true;
                }
            }
            else if ( gameEnding ) {
                // proper ending
            }
        }
        else if ( ms.getStatus() != sf::Sound::Playing ) {
            if ( musicCount < 1 ) {
                musicCount++;
                ms.setBuffer(msba);
                ms.play();
            }
            else if ( musicCount < 3 ) {
                musicCount++;
                ms.setBuffer(msbb);
                ms.play();
            }
            else if ( musicCount == 3 ) {
                musicCount++;
                ms.setBuffer(msba);
                ms.play();
            }
            else if ( musicCount == 4 ) {
                musicCount++;
                ms.setBuffer(msbc);
                ms.play();
            }
            else if ( musicCount == 5 ) {
                musicCount = 0;
                ms.setBuffer(msba);
                ms.play();
            }
        }

        // handle input
        vMoving = false;
        hMoving = false;
        if ( gameRunning ) {
            if ( sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ) {
                plS.move( sf::Vector2f(moveRate,0.f) * timeDelta );
                faceLeft = false;
                hMoving = true;
            }
            else if ( sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ) {
                plS.move( sf::Vector2f(-moveRate,0.f) * timeDelta );
                faceLeft = true;
                hMoving = true;
            }
            if ( sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ) {
                plS.move( sf::Vector2f(0.f,-moveRate) * timeDelta );
                vMoving = true;
            }
            else if ( sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ) {
                plS.move( sf::Vector2f(0.f,moveRate) * timeDelta );
                vMoving = true;
            }
            if ( playerFireReady ) {
                if ( sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl) ) {
                    if ( ammoLeft > 0 ) {
                        playerFireReady = false;
                        sShot.play();
                        playerFiring = true;
                    }
                    else {
                        if ( playerWeapon == 0 )
                            sClick.play();
                        else
                            sClickBack.play();
                    }
                }
            }
        }
        else if ( sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) ) {
            sClickBack.play();
            rWin.close();
        }

        // handle move limits
        sf::Vector2f mv( plS.getPosition() );
        if ( mv.y < 190.f ) {
            mv.y = 190.f;
            vMoving = false;
        }
        else if ( mv.y > 280.f ) {
            mv.y = 280.f;
            vMoving = false;
        }
        if ( mv.x < 40.f ) {
            mv.x = 40.f;
            hMoving = false;
        }
        else if ( mv.x > 660.f ) {
            mv.x = 660.f;
            hMoving = false;
        }
        plS.setPosition(mv);

        // handle view scroll
        sf::Vector2f vCenter( mnVw.getCenter() );
        if ( vCenter.x < 700.f && mv.x > (vCenter.x-200.f) ) {
            mnVw.move( viewMoveRate * timeDelta, 0.f );
            hudAmmo.move( viewMoveRate * timeDelta, 0.f );
            hudScore.move( viewMoveRate * timeDelta, 0.f );
            hudHealth.move( viewMoveRate * timeDelta, 0.f );
            endText.move( viewMoveRate * timeDelta, 0.f );
        }
        else if ( vCenter.x > 500.f && mv.x < (vCenter.x-300.f) ) {
            mnVw.move( -viewMoveRate * timeDelta, 0.f );
            hudAmmo.move( -viewMoveRate * timeDelta, 0.f );
            hudScore.move( -viewMoveRate * timeDelta, 0.f );
            hudHealth.move( -viewMoveRate * timeDelta, 0.f );
            endText.move( -viewMoveRate * timeDelta, 0.f );
        }

        // handle avatar
        animTm += clk.restart();
        if ( !vMoving && !hMoving ) {
            plS.setTextureRect( sf::IntRect( 720, (playerAnimOffset*playerWeapon)-4, 360, 360 ) );
        }
        if ( ( vMoving || hMoving ) && animTm > sf::seconds(frameRate) ) {
            animTm = sf::seconds(0.f);
            frameNum++;
            if ( frameNum % 2 == 0 ) {
                plS.setTextureRect( sf::IntRect( -18, (playerAnimOffset*playerWeapon)-4, 360, 360 ) );
            }
            else {
                plS.setTextureRect( sf::IntRect( 360, (playerAnimOffset*playerWeapon)-4, 360, 360 ) );
            }
        }
        if ( faceLeft ) {
            plS.setScale(-.1f, .1f);
        }
        else {
            plS.setScale(.1f, .1f);
        }

        // handle shots
        if ( playerFiring ) {
            playerFiring = false;
            // spawn shots
            int numShots = 1;
            if ( playerWeapon == 1 )
                numShots = 3;
            for ( int i=0; i<numShots; i++ ) {
                ammoLeft--;
                shots[shotIndex].proj.setPosition( plS.getPosition() );
                if ( playerWeapon == 0 )
                    shots[shotIndex].proj.move( sf::Vector2f(0.f,-26.f) );
                else
                    shots[shotIndex].proj.move( sf::Vector2f(0.f,-20.f) );
                fireMoveLeft = faceLeft;
                if ( fireMoveLeft ) {
                    shots[shotIndex].proj.move(-14.f,0.f);
                    shots[shotIndex].fireVector = sf::Vector2f( -fireSpeed, 0.f );
                }
                else {
                    shots[shotIndex].proj.move(14.f,0.f);
                    shots[shotIndex].fireVector = sf::Vector2f( fireSpeed, 0.f );
                }
                if ( playerWeapon == 1 && i == 0 )
                    shots[shotIndex].fireVector *= 1.1f;
                if ( i == 1 ) {
                    shots[shotIndex].fireVector += sf::Vector2f( 0.f, (-fireSpeed/4) );
                }
                else if ( i == 2 ) {
                    shots[shotIndex].fireVector += sf::Vector2f( 0.f, (fireSpeed/4) );
                }
                shots[shotIndex].fireLife.restart();
                shotIndex++;
                if ( shotIndex >= shotMax )
                    shotIndex = 0;
            }
            fireClock.restart();
        }
        for ( int i=0; i<8; i++ ) {
            if ( shots[i].proj.getPosition().x > 0.f ) {
                // shot movement
                if ( shots[i].proj.getPosition().x != 0.f ) {
                    shots[i].proj.move( shots[i].fireVector * timeDelta );
                }
                // shot impact
                for ( int z=0; z<20; z++ ) {
                    if ( hoard[z].inPlay ) {
                        if ( shots[i].proj.getGlobalBounds().intersects( hoard[z].zS.getGlobalBounds() ) ) {
                            shots[i].proj.setPosition( sf::Vector2f(-10.f,-10.f) );
                            hoard[z].health--;
                            hoard[z].zS.setTextureRect( sf::IntRect( (82+((4-hoard[z].health)*148)),464,132,132 ) );
                            if ( hoard[z].health == 3 ) {
                                hoard[z].dying = true;
                                hoard[z].animTimer.restart();
                            }
                            sImpact.play();
                        }
                    }
                }
                // handle shot life end
                if ( shots[i].fireLife.getElapsedTime() > sf::seconds(shotLifeTime) ) {
                    // clear shots
                    shots[i].proj.setPosition( sf::Vector2f(-10.f,-10.f) );
                }
            }
        }
        if ( fireClock.getElapsedTime() > sf::seconds(reFireRate) ) {
            playerFireReady = true;
        }

        // handle zombies
        for ( int i=0; i<20; i++ ) {
            if ( ( playerWon || gameRunning ) && hoard[i].inPlay ) {
                if ( !hoard[i].dying ) {
                    if ( hoard[i].facingLeft ) {
                        hoard[i].zS.setScale(-0.25f,0.25f);
                    }
                    else {
                        hoard[i].zS.setScale(0.25f,0.25f);
                    }
                    // close in on player
                    bool zMoveV = false;
                    bool zMoveH = false;
                    if ( hoard[i].zS.getPosition().x > plS.getPosition().x+10.f ) {
                        hoard[i].zS.move(-hoardMoveRate * timeDelta,0.f);
                        hoard[i].facingLeft = true;
                        zMoveH = true;
                    }
                    else if ( hoard[i].zS.getPosition().x < plS.getPosition().x-10.f ) {
                        hoard[i].zS.move(hoardMoveRate * timeDelta,0.f);
                        hoard[i].facingLeft = false;
                        zMoveH = true;
                    }
                    // closer vertically if close
                    if ( std::abs( static_cast<long>( hoard[i].zS.getPosition().x - plS.getPosition().x ) ) < 80.f ) {
                        if ( hoard[i].zS.getPosition().y > plS.getPosition().y+10.f ) {
                            hoard[i].zS.move(0.f,-(hoardMoveRate/3.f) * timeDelta);
                            zMoveV = true;
                        }
                        else if ( hoard[i].zS.getPosition().y < plS.getPosition().y-10.f ) {
                            hoard[i].zS.move(0.f,(hoardMoveRate/3.f) * timeDelta);
                            zMoveV = true;
                        }
                    }
                    // handle animation
                    if ( !hoard[i].dying ) {
                        if ( !zMoveV && !zMoveH ) {
                            hoard[i].animFrame = 0;
                            hoard[i].zS.setTextureRect( sf::IntRect(0,168,132,132) );
                        }
                        else {
                            int cycle = ( hoard[i].animTimer.getElapsedTime().asSeconds() * 2.f );
                            hoard[i].animFrame = (cycle % 4);
                            hoard[i].zS.setTextureRect( sf::IntRect(0+(148*hoard[i].animFrame),8,132,132) );
                        }
                    }
                    // harm player
                    if ( hoard[i].attacking && hoard[i].attackTimer.getElapsedTime() > sf::seconds(1.f) ) {
                        hoard[i].attacking = false;
                        health--;
                        playerHurt = true;
                        hudTimer.restart();
                        if ( health <= 0 ) {
                            // lose
                            gameRunning = false;
                            playerWon = false;
                            health = 0;
                        }
                    }
                    if ( !hoard[i].attacking && hoard[i].zS.getGlobalBounds().intersects( plS.getGlobalBounds() ) ) {
                        hoard[i].attacking = true;
                        hoard[i].attackTimer.restart();
                    }
                }
                else {
                    // handle death frame
                    hoard[i].animFrame = ( hoard[i].animTimer.getElapsedTime().asSeconds() * 5.f );
                    hoard[i].zS.setTextureRect( sf::IntRect( (148+(hoard[i].animFrame*148)),168,132,132 ) );
                    if ( hoard[i].animTimer.getElapsedTime().asSeconds() > 0.6f ) {
                        hoard[i].dying = false;
                        hoard[i].inPlay = false;
                        if ( gameRunning )
                            score += 5;
                        playerScored = true;
                        hudTimer.restart();
                        hoard[i].zS.setPosition(-40.f,-20.f);
                        int spawnNum = (rand()%2)+1;
                        for ( int s=0; s<spawnNum; s++ ) {
                            int n = ( rand() % 20 );
                            int safety = 0;
                            while ( safety < 20 && ( n == i || hoard[n].inPlay ) ) {
                                n = ( rand() % 20 );
                                safety++;
                            }
                            if ( safety < 20 ) {
                                if ( rand()%2==0 )
                                    hoard[n].zS.setPosition( -80.f, 185.f );
                                else
                                    hoard[n].zS.setPosition( 780.f, 185.f );
                                hoard[n].zS.move( (rand()%30)-15.f, (rand()%8)-4.f+(n*4.f)-4.f ); // pre-sorted zombie layers
                                hoard[n].zS.setTextureRect( sf::IntRect( 0,8,132,132 ) );
                                hoard[n].animTimer.restart();
                                hoard[n].health = 4;
                                hoard[n].attacking = false;
                                hoard[n].animFrame = 0;
                                hoard[n].animTimer.restart();
                                hoard[n].inPlay = true;
                            }
                        }
                        // check player win
                        if ( score >= scoreToWin ) {
                            playerWon = true;
                            gameRunning = false;
                            endText.setString("YOU WIN!");
                            for ( int i=0; i<20; i++ ) {
                                if ( hoard[i].inPlay && !hoard[i].dying ) {
                                    hoard[i].dying = true;
                                    hoard[i].animFrame = 0;
                                    hoard[i].animTimer.restart();
                                }
                            }
                            hudTimer.restart();
                        }
                    }
                }
            }
        }

        // handle pickup
        if ( pickupLive ) {
            sf::FloatRect puBnd;
            puBnd = puS.getGlobalBounds();
            sf::FloatRect plBnd;
            plBnd = plS.getGlobalBounds();
            if ( puBnd.intersects( plBnd ) ) {
                if ( pickupType == 0 ) {
                    shotIndex = 0;
                    playerWeapon = 0;
                    reFireRate = 0.5f;
                    ammoLeft = 12;
                }
                else {
                    shotIndex = 1;
                    playerWeapon = 1;
                    reFireRate = 1.f;
                    ammoLeft = 15;
                }
                pickupLive = false;
                // SFX pickup
                sPickup.play();
                pickupTimer.restart();
            }
        }

        if ( gameRunning && !pickupLive && pickupTimer.getElapsedTime() > sf::seconds(8.f) ) {
            pickupType = ( rand() % 2 );
            if ( pickupType == 0 ) {
                puS.setTextureRect( sf::IntRect(0,0,256,256) );
            }
            else {
                puS.setTextureRect( sf::IntRect(256,0,256,256) );
            }
            puS.setPosition(sf::Vector2f(100.f, 200.f));
            if ( ( rand() % 2 ) == 0 ) {
                puS.move( 0.f, 50.f );
            }
            else {
                puS.move( 480.f, 50.f );
            }
            pickupLive = true;
        }

        rWin.clear();

        rWin.setView(mnVw);
        rWin.draw(bgS);

        if ( pickupLive )
            rWin.draw(puS);

        for ( int i=0; i<20; i++ ) {
            // simplified sorting
            if ( hoard[i].inPlay && hoard[i].zS.getPosition().y <= plS.getPosition().y-2.f ) {
                rWin.draw(hoard[i].zS);
            }
        }

        rWin.draw(plS);

        for ( int i=0; i<20; i++ ) {
            if ( hoard[i].inPlay && hoard[i].zS.getPosition().y > plS.getPosition().y-2.f ) {
                rWin.draw(hoard[i].zS);
            }
        }

        for ( int i=0; i<8; i++ ) {
            if ( shots[i].proj.getPosition().x > 0.f )
                rWin.draw(shots[i].proj);
        }

        // HUD
        char* iv = new char[5];
        std::string hudLabel = "AMMO x";
        sprintf( iv, "%d", ammoLeft );
        hudAmmo.setString(hudLabel+iv);
        hudLabel = "SCORE ";
        sprintf( iv, "%d", score );
        hudScore.setString(hudLabel+iv);
        hudLabel = "HEALTH ";
        sprintf( iv, "%d", health );
        hudHealth.setString(hudLabel+iv);
        // drop shadow text
        hudAmmo.setFillColor(sf::Color::Black);
        hudScore.setFillColor(sf::Color::Black);
        hudHealth.setFillColor(sf::Color::Black);
        hudAmmo.move(1.f,1.f);
        hudScore.move(1.f,1.f);
        hudHealth.move(1.f,1.f);
        rWin.draw(hudAmmo);
        rWin.draw(hudScore);
        rWin.draw(hudHealth);
        if ( gameEnding ) {
            endText.setFillColor(sf::Color::Black);
            endText.move(1.f, 1.f);
            rWin.draw(endText);
            endText.setFillColor(sf::Color::White);
        }
        // text
        hudAmmo.setFillColor(sf::Color::White);
        hudScore.setFillColor(sf::Color::White);
        hudHealth.setFillColor(sf::Color::White);
        if ( hudTimer.getElapsedTime().asSeconds() < .5f ) {
            if ( gameRunning ) {
                if ( ammoLeft == 0 )
                    hudAmmo.setFillColor(sf::Color::Yellow);
                if ( playerScored )
                    hudScore.setFillColor(sf::Color::Blue);
                if ( playerHurt )
                    hudHealth.setFillColor(sf::Color::Red);
            }
            if ( playerWon )
                endText.setFillColor(sf::Color::Green);
            else
                endText.setFillColor(sf::Color::Red);
        }
        else {
            if ( ammoLeft == 0 )
                hudAmmo.setFillColor(sf::Color::White);
            if ( playerScored ) {
                playerScored = false;
                hudScore.setFillColor(sf::Color::White);
            }
            if ( playerHurt ) {
                playerHurt = false;
                hudHealth.setFillColor(sf::Color::White);
            }
            endText.setFillColor(sf::Color::White);
        }
        if ( hudTimer.getElapsedTime().asSeconds() > 1.f ) {
            hudTimer.restart();
        }
        hudAmmo.move(-1.f,-1.f);
        hudScore.move(-1.f,-1.f);
        hudHealth.move(-1.f,-1.f);
        rWin.draw(hudAmmo);
        rWin.draw(hudScore);
        rWin.draw(hudHealth);
        if ( gameEnding ) {
            // end text
            endText.move(-1.f,-1.f);
            rWin.draw(endText);
        }

        rWin.display();
    }

    return soundOkay;
}
