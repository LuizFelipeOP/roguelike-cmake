#pragma once
class Player;


class Observer {
    public:
        virtual void onNotify(Player& player) = 0;
        
        virtual ~Observer() = default;
};
