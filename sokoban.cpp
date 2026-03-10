//Uwaga! Co najmniej C++17!!!
//Project-> ... Properties->Configuration Properties->General->C++ Language Standard = ISO C++ 17 Standard (/std:c++17)

#include "SFML/Graphics.hpp"
#include <fstream>

enum class Field { VOID, FLOOR, WALL, BOX, PARK, PLAYER };

class Sokoban : public sf::Drawable
{
public:
	void LoadMapFromFile(std::string fileName);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void SetDrawParameters(sf::Vector2u draw_area_size);
	void Move_Player_Left();
	void Move_Player_Right();
	void Move_Player_Up();
	void Move_Player_Down();
	bool Is_Victory() const;

private:
	std::vector<std::vector<Field>> map;
	sf::Vector2f shift, tile_size;
	sf::Vector2i player_position;
	std::vector<sf::Vector2i> park_positions;

	void move_player(int dx, int dy);
};

void Sokoban::LoadMapFromFile(std::string fileName)
{
	std::string str;
	std::vector<std::string> vos;

	std::ifstream in(fileName.c_str());
	if (!in)
	{
		//OH NO! Anyway....
	}
	while (std::getline(in, str)) { vos.push_back(str); }
	in.close();

	map.clear();
	map.resize(vos.size(), std::vector<Field>(vos[0].size()));
	for (auto [row, row_end, y] = std::tuple{ vos.cbegin(), vos.cend(), 0 }; row != row_end; ++row, ++y)
		for (auto [element, end, x] = std::tuple{ row->begin(), row->end(), 0 }; element != end; ++element, ++x)
			switch (*element)
			{
			case 'X': map[y][x] = Field::WALL; break;
			case '*': map[y][x] = Field::VOID; break;
			case ' ': map[y][x] = Field::FLOOR; break;
			case 'B': map[y][x] = Field::BOX; break;
			case 'P': map[y][x] = Field::PARK; park_positions.push_back(sf::Vector2i(x, y));  break;
			case 'S': map[y][x] = Field::PLAYER; player_position = sf::Vector2i(x, y);  break;
			}
}

void Sokoban::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// Tu niew�tpliwie powinno co� by� : -) Tu nale�y narysowa� wszystko. O tak jako� :
	//target.draw(....);

	//Przydatna mo�e by� p�tla :
	for (int y = 0; y < map.size(); ++y)
		for (int x = 0; x < map[y].size(); ++x)
		{
			sf::RectangleShape rec(this->tile_size);
			//Teraz map[y][x] m�wi nam CO mamy narysowa�.
			

			if (map[y][x] == Field::WALL) {
				rec.setFillColor(sf::Color(181, 101, 29));
			}
			else if (map[y][x] == Field::VOID) {
				rec.setFillColor(sf::Color::Black);
			}
			else if (map[y][x] == Field::FLOOR) {
				rec.setFillColor(sf::Color::White);
			}
			else if (map[y][x] == Field::BOX) {
				rec.setFillColor(sf::Color(101, 67, 33));
			}
			else if (map[y][x] == Field::PARK) {
				rec.setFillColor(sf::Color(50, 205, 50));
			}
			else if (map[y][x] == Field::PLAYER) {
				rec.setFillColor(sf::Color::Blue);
			}
			rec.setOutlineColor(sf::Color(60, 60, 60));
			rec.setOutlineThickness(1.f);
			rec.setPosition(sf::Vector2f(
				this->shift.x + x * this->tile_size.x,
				this->shift.y + y * this->tile_size.y
			));
			target.draw(rec, states);
		}
}

void Sokoban::SetDrawParameters(sf::Vector2u draw_area_size)
{
	this->tile_size = sf::Vector2f(
		std::min(std::floor((float)draw_area_size.x / (float)map[0].size()), std::floor((float)draw_area_size.y / (float)map.size())),
		std::min(std::floor((float)draw_area_size.x / (float)map[0].size()), std::floor((float)draw_area_size.y / (float)map.size()))
	);
	this->shift = sf::Vector2f(
		((float)draw_area_size.x - this->tile_size.x * map[0].size()) / 2.0f,
		((float)draw_area_size.y - this->tile_size.y * map.size()) / 2.0f
	);
}

void Sokoban::Move_Player_Left()
{
	move_player(-1, 0);
}

void Sokoban::Move_Player_Right()
{
	move_player(1, 0);
}

void Sokoban::Move_Player_Up()
{
	move_player(0, -1);
}

void Sokoban::Move_Player_Down()
{
	move_player(0, 1);
}

void Sokoban::move_player(int dx, int dy)
{
	bool allow_move = false; // Pesymistyczne za��my, �e gracz nie mo�e si� poruszy�.
	sf::Vector2i new_pp(player_position.x + dx, player_position.y + dy); //Potencjalna nowa pozycja gracza.
	Field fts = map[new_pp.y][new_pp.x]; //Element na miejscu na kt�re gracz zamierza przej��.
	Field ftsa = map[new_pp.y + dy][new_pp.x + dx]; //Element na miejscu ZA miejscem na kt�re gracz zamierza przej��. :-D

	//Gracz mo�e si� poruszy� je�li pole na kt�rym ma stan�� to pod�oga lub miejsce na skrzynki.
	if (fts == Field::FLOOR || fts == Field::PARK) allow_move = true;
	//Je�li pole na kt�re chce si� poruszy� gracz zawiera skrzynk� to mo�e si� on poruszy� jedynie je�li kolejne pole jest puste lub zawiera miejsce na skrzynk�  - bo wtedy mo�e przepchn�� skrzynk�.
	if (fts == Field::BOX && (ftsa == Field::FLOOR || ftsa == Field::PARK))
	{
		allow_move = true;
		//Przepychamy skrzynk�.
		map[new_pp.y + dy][new_pp.x + dx] = Field::BOX;
		//Oczywi�cie pole na kt�rym sta�a skrzynka staje si� teraz pod�og�.
		map[new_pp.y][new_pp.x] = Field::FLOOR;
	}

	if (allow_move)
	{
		//Przesuwamy gracza.
		map[player_position.y][player_position.x] = Field::FLOOR;
		player_position = new_pp;
		map[player_position.y][player_position.x] = Field::PLAYER;
	}

	//Niestety w czasie ruchu mog�y �ucierpie� miejsca na skrzynk�. ;-(
	for (auto park_position : park_positions) if (map[park_position.y][park_position.x] == Field::FLOOR) map[park_position.y][park_position.x] = Field::PARK;
}

bool Sokoban::Is_Victory() const
{
	//Tym razem dla odmiany optymistycznie zak�adamy, �e gracz wygra�.
	//No ale je�li na kt�rymkolwiek miejscu na skrzynki nie ma skrzynki to chyba za�o�enie by�o zbyt optymistyczne... : -/
	for (auto park_position : park_positions) if (map[park_position.y][park_position.x] != Field::BOX) return false;
	return true;
}

int main()
{
	sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "IUiBG Lab 01", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	Sokoban sokoban;
	std::optional<sf::Event> event;

	sokoban.LoadMapFromFile("plansza.txt");
	sokoban.SetDrawParameters(window.getSize());

	while (window.isOpen())
	{

		while (event = window.pollEvent())
		{
			if (const auto resized = event->getIf<sf::Event::Resized>())
			{
				window.setView(sf::View(sf::FloatRect({ 0, 0 }, static_cast<sf::Vector2f>(resized->size))));
				sokoban.SetDrawParameters(window.getSize());
			}
			//Oczywi�cie tu powinny zosta� jako� obs�u�one inne zdarzenia.Na przyk�ad jak gracz naci�nie klawisz w lewo powinno pojawi� si� wywo�anie metody :
			//sokoban.Move_Player_Left();
			//W dowolnym momencie mog� Pa�stwo sprawdzi� czy gracz wygra�:
			//sokoban.Is_Victory();
			else if (const auto key = event->getIf<sf::Event::KeyPressed>()) // <-- obsługa klawiatury
			{
				switch (key->code)
				{
				case sf::Keyboard::Key::Left:  sokoban.Move_Player_Left(); break;
				case sf::Keyboard::Key::Right: sokoban.Move_Player_Right(); break;
				case sf::Keyboard::Key::Up:    sokoban.Move_Player_Up(); break;
				case sf::Keyboard::Key::Down:  sokoban.Move_Player_Down(); break;
				case sf::Keyboard::Key::Escape: window.close(); break;
				default: break;
				}

				if (sokoban.Is_Victory())
					window.setTitle("Zwycięstwo! :)");
			}

			else if (event->getIf<sf::Event::Closed>())
			{
				window.close();
			}
		}
		window.clear();
		window.draw(sokoban);
		window.display();
	}

	return 0;
}