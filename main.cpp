#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode{ 500, 500 }, "durak");
    window.setFramerateLimit(144);

    while (window.isOpen())
    {
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear();
        window.display();
    }
}