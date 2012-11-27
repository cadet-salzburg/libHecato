#ifndef HEADER_F57822AC45837283
#define HEADER_F57822AC45837283

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <XnCppWrapper.h>
#include <unistd.h>
#include "HTMutex.h"


#include "HTNetworkSender.h"
#include "HTNetworkReceiver.h"
#include "HTTUIOServer.h"
#include "HTBlobInterpreterMulti.h"

class HTApp : public sf::RenderWindow, public HTBlobInterpreterMulti
{
public:
    struct BlobDisplay
    {
        sf::Vector2f vec;
        uint32_t id;
        HTIBlobResultTarget::BlobResultType brtype;
        BlobDisplay(float x, float y, uint32_t _id, HTIBlobResultTarget::BlobResultType t) : vec(x,y), id(_id), brtype(t) {}
    };

    HTApp();
    ~HTApp();

    void lockAndDrawAll();
    void unlock();
    void update();
    void handleEvents(const std::vector<HTBlobInterpreter::BlobRecord>& events);
    void handleBlobResult(std::vector<HTIBlobResultTarget::SBlobResult>& points, int id);
    void onResize();

private:
    sf::CircleShape circle;
    sf::RectangleShape rectShape;
    xn::Context ctx;
    std::vector<HTDeviceThreaded*> ktt;
    HTMutex drawMutti;
    std::vector<BlobDisplay> drawBlobs;

    HTNetworkSender* sender;
    HTNetworkReceiver* recv;
    HTTUIOServer* tuioServer;
    float winW, winH;

};

#endif // header guard
