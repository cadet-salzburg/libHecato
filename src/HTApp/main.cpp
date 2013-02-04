#include "HTApp.h"
#include "HTAppSettings.h"

int main()
{


    HTApp app;

    /*Some setups (usually with KVM switches) seem to screw up the vsync with the monitor
      causing strange app lockups. For this case, syncing to v is configurable in the appsettings.xml*/
    bool useVSync = HTAppSettings::useFPS();
    const int fDelay = HTAppSettings::getFPSDelay() * 1000;

	app.setVerticalSyncEnabled(useVSync);

    // Start the game loop
    while (app.isOpen())
    {
        // Process events
        sf::Event Event;
        while (app.pollEvent(Event))
        {
            // Close window : exit
            if (Event.type == sf::Event::Closed)
                app.close();
        }

        // Clear screen
        app.clear(sf::Color(0, 0, 255));

        // Draw the sprite
        app.update();


        app.lockAndDrawAll();
        app.display();
        app.unlock();
#ifndef WIN32
        if (!useVSync)
            usleep(fDelay);
#endif
    }
    return EXIT_SUCCESS;
}

