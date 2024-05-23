#include <chrono>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>


// Dinh nghia cac hang so trong game
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float PADDLE_SPEED = 1.0f;
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 100;
const float BALL_SPEED = 0.75f;
const int BALL_WIDTH = 15;
const int BALL_HEIGHT = 15;


// Liet ke cac nut dieu khien
enum Buttons
{
	PaddleOneUp = 0,
	PaddleOneDown,
	PaddleTwoUp,
	PaddleTwoDown,
};


// Liet ke cac loai va cham
enum class CollisionType
{
	None,
	Top,
	Middle,
	Bottom,
	Left,
	Right
};


//
struct Contact
{
	CollisionType type; // Loai va cham
	float penetration; // Độ sâu cua va cham
};


class Vec2 // 1 class bieu dien vector trong khong gian 2 chieu
{
public:
	Vec2()
		: x(0.0f), y(0.0f)
	{}

	Vec2(float x, float y)
		: x(x), y(y)
	{}

	Vec2 operator+(Vec2 const& rhs)
	{
		return Vec2(x + rhs.x, y + rhs.y); // phep cong vector
	}

	Vec2& operator+=(Vec2 const& rhs) // phep cong gan vector
	{
		x += rhs.x;
		y += rhs.y;

		return *this;
	}

	Vec2 operator*(float rhs) // phep nhan vector vs 1 so vo huong
	{
		return Vec2(x * rhs, y * rhs);
	}

	float x, y;
};


class Paddle // Dinh nghia Paddle
{
public:
	Paddle(Vec2 position, Vec2 velocity) // constructor co 2 tham so khoi tao vi tri va van toc paddle
		: position(position), velocity(velocity) // gan gia tri tham so cho position va velocity
	{
		rect.x = static_cast<int>(position.x); // chuyen doi x va y tu float sang int de gan cho thuoc tinh rect.x va rect.y
		rect.y = static_cast<int>(position.y);
		rect.w = PADDLE_WIDTH;
		rect.h = PADDLE_HEIGHT;
	}

	void Update(float dt) // Cap nhat vi tri cua paddle dua vao van toc va thoi gian troi qua dt
	{
		position += velocity * dt;

		if (position.y < 0)
		{
			// Gioi han o dinh man hinh
			position.y = 0;
		}
		else if (position.y > (WINDOW_HEIGHT - PADDLE_HEIGHT))
		{
			// Gioi han o cuoi man hinh
			position.y = WINDOW_HEIGHT - PADDLE_HEIGHT;
		}
	}

	void Draw(SDL_Renderer* renderer)
	{
		rect.y = static_cast<int>(position.y);

		SDL_RenderFillRect(renderer, &rect);
	}

	Vec2 position;
	Vec2 velocity;
	SDL_Rect rect{};
};


class Ball // Dinh nghia Ball
{
public:
	Ball(Vec2 position, Vec2 velocity) // constructor co 2 tham so khoi tao vi tri va van toc ball
		: position(position), velocity(velocity) // gan gia tri tham so cho position va velocity
	{
		rect.x = static_cast<int>(position.x); // chuyen doi x va y tu float sang int de gan cho thuoc tinh rect.x va rect.y
		rect.y = static_cast<int>(position.y);
		rect.w = BALL_WIDTH;
		rect.h = BALL_HEIGHT;
	}


	void Update(float dt) // Cap nhat vi tri cua ball dua vao van toc va thoi gian troi qua dt
	{
		position += velocity * dt; // Cap nhat vi tri = Velocity * Thoi gian troi qua dt
	}


	void Draw(SDL_Renderer* renderer)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);

		SDL_RenderFillRect(renderer, &rect);
	}


	void CollideWithPaddle(Contact const& contact) // XU LI VA CHAM PADDLE VA BALL
	{
		position.x += contact.penetration; // Cap nhat vi tri ball theo penetration cua va cham va dao ngc huong van toc
		velocity.x = -velocity.x;

		if (contact.type == CollisionType::Top) // Neu va cham dc tinh la Contact Top
		{
			velocity.y = -.75f * BALL_SPEED;
		}
		else if (contact.type == CollisionType::Bottom) // Neu va cham dc tinh la Contact Bottom
		{
			velocity.y = 0.75f * BALL_SPEED;
		}
	}


	void CollideWithWall(Contact const& contact) // XU LI VA CHAM BALL VA WALL
	{
		if ((contact.type == CollisionType::Top) // Va cham dc tinh la Top hoac Bottom => Cap nhat theo penetration cua va cham , dao ngc van toc y.
		    || (contact.type == CollisionType::Bottom))
		{
			position.y += contact.penetration;
			velocity.y = -velocity.y;
		}
		else if (contact.type == CollisionType::Left) // Va cham dc tinh la LEFT hoac RIGHT => GHI DIEM => Respawn lai ball ve giua man hinh , cho ball nga ve ng thang
		{
			position.x = WINDOW_WIDTH / 2.0f;
			position.y = WINDOW_HEIGHT / 2.0f;
			velocity.x = BALL_SPEED;
			velocity.y = 0.75f * BALL_SPEED;
		}
		else if (contact.type == CollisionType::Right)
		{
			position.x = WINDOW_WIDTH / 2.0f;
			position.y = WINDOW_HEIGHT / 2.0f;
			velocity.x = -BALL_SPEED;
			velocity.y = 0.75f * BALL_SPEED;
		}
	}

	Vec2 position;
	Vec2 velocity;
	SDL_Rect rect{};
};


class PlayerScore // DIEM SO TRONG TRO CHOI
{
public:
	PlayerScore(Vec2 position, SDL_Renderer* renderer, TTF_Font* font)
		: renderer(renderer), font(font)
	{
		surface = TTF_RenderText_Solid(font, "0", {0xFF, 0xFF, 0xFF, 0xFF}); // Render len surface vs mau trang
		texture = SDL_CreateTextureFromSurface(renderer, surface); // Tao texture tu surface de ve len ma hinh

		int width, height;
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height); // Lay kich thuoc cua texture

		rect.x = static_cast<int>(position.x); // chuyen tu float sang int de gan rect.x va rect.y
		rect.y = static_cast<int>(position.y);
		rect.w = width; // dat chieu rong va chieu cao dua tren kich thuoc cua texture
		rect.h = height;
	}

	~PlayerScore()
	{
		SDL_FreeSurface(surface); // Giai phong surface , texture
		SDL_DestroyTexture(texture);
	}


	void SetScore(int score)
	{
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);

		surface = TTF_RenderText_Solid(font, std::to_string(score).c_str(), {0xFF, 0xFF, 0xFF, 0xFF}); // Render diem so moi len surface
		texture = SDL_CreateTextureFromSurface(renderer, surface); // Tao texture moi tu surface

		int width, height;
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height); // Lay kich thuoc texture moi va cap nhat
		rect.w = width;
		rect.h = height;
	}


	void Draw()
	{
		SDL_RenderCopy(renderer, texture, nullptr, &rect); // Ve diem so len man hinh
	}

	SDL_Renderer* renderer;
	TTF_Font* font;
	SDL_Surface* surface{};
	SDL_Texture* texture{};
	SDL_Rect rect{};
};


Contact CheckPaddleCollision(Ball const& ball, Paddle const& paddle) // KIEM TRA VA CHAM GIUA PADDLE VA BALL , TRA VE LOAI VA CHAM VA DO XUYEN THAU VA CHAM
{
	float ballLeft = ball.position.x;
	float ballRight = ball.position.x + BALL_WIDTH;
	float ballTop = ball.position.y;
	float ballBottom = ball.position.y + BALL_HEIGHT;

	float paddleLeft = paddle.position.x;
	float paddleRight = paddle.position.x + PADDLE_WIDTH;
	float paddleTop = paddle.position.y;
	float paddleBottom = paddle.position.y + PADDLE_HEIGHT;

	Contact contact{}; // LUU THONG TIN VE VA CHAM

	if (ballLeft >= paddleRight)
	{
		return contact;
	}

	if (ballRight <= paddleLeft)
	{
		return contact;
	}

	if (ballTop >= paddleBottom)
	{
		return contact;
	}

	if (ballBottom <= paddleTop)
	{
		return contact;
	}

	float paddleRangeUpper = paddleBottom - (2.0f * PADDLE_HEIGHT / 3.0f); // Vi tri 1/3 trên của paddle
	float paddleRangeMiddle = paddleBottom - (PADDLE_HEIGHT / 3.0f);// Vị trí 1/3 giữa của paddle

	if (ball.velocity.x < 0) // Vận tốc hướng trái
	{
		// Left paddle
		contact.penetration = paddleRight - ballLeft; // Tính độ xuyên thấu tyuwf cạnh phải Paddle đến Cạnh trái của bóng
	}
	else if (ball.velocity.x > 0)
	{
		// Right paddle
		contact.penetration = paddleLeft - ballRight;
	}

	if ((ballBottom > paddleTop)
	    && (ballBottom < paddleRangeUpper)) // Đáy của quả bóng nằm trong phần 3 trên của gậy => TOP
	{
		contact.type = CollisionType::Top;
	}
	else if ((ballBottom > paddleRangeUpper)
	         && (ballBottom < paddleRangeMiddle)) // Đáy của bóng nằm trong phần 3 giữa của gậy => MIDDLE
	{
		contact.type = CollisionType::Middle;
	}
	else
	{
		contact.type = CollisionType::Bottom; // Còn lại là Bottom
	}

	return contact;
}


Contact CheckWallCollision(Ball const& ball) // KIEM TRA VA CHAM GIUA BALL VA WALL
{
	float ballLeft = ball.position.x; // Lưu trữ tọa độ
	float ballRight = ball.position.x + BALL_WIDTH;
	float ballTop = ball.position.y;
	float ballBottom = ball.position.y + BALL_HEIGHT;

	Contact contact{};

	if (ballLeft < 0.0f) // Cạnh trái của bóng vượt qua biên màn hình trái
	{
		contact.type = CollisionType::Left;
	}
	else if (ballRight > WINDOW_WIDTH)
	{
		contact.type = CollisionType::Right;
	}
	else if (ballTop < 0.0f) // Cạnh trên của bóng vượt biên màn hình trên => Loại va chạm Top
	{
		contact.type = CollisionType::Top;
		contact.penetration = -ballTop; // Độ xuyên thấu là tọa độ âm
	}
	else if (ballBottom > WINDOW_HEIGHT) // Cạnh dưới Ball vượt biên màn hình dưới => Loại va chạm Bottom
	{
		contact.type = CollisionType::Bottom;
		contact.penetration = WINDOW_HEIGHT - ballBottom; // Độ xuyên thấu
	}

	return contact;
}


int main(int argc, char* argv[])
{
	// Khởi tạo thành phần SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	TTF_Init();


	// Tạo cửa sổ và renderer
	SDL_Window* window = SDL_CreateWindow("Pong", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);


	// Khởi tạo font
	TTF_Font* scoreFont = TTF_OpenFont("Peepo.ttf", 40);




	// Logic Game
	{
		// Tạo chỗ điền điểm
		PlayerScore playerOneScoreText(Vec2(WINDOW_WIDTH / 4, 20), renderer, scoreFont);

		PlayerScore playerTwoScoreText(Vec2(3 * WINDOW_WIDTH / 4, 20), renderer, scoreFont);


		// Tạo BALL
		Ball ball(
			Vec2(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f),
			Vec2(BALL_SPEED, 0.0f));


		// Tạo 2 PADDLES
		Paddle paddleOne(
			Vec2(50.0f, WINDOW_HEIGHT / 2.0f),
			Vec2(0.0f, 0.0f));

		Paddle paddleTwo(
			Vec2(WINDOW_WIDTH - 50.0f, WINDOW_HEIGHT / 2.0f),
			Vec2(0.0f, 0.0f));


		// Biến và vòng lặp chính
		int playerOneScore = 0;
		int playerTwoScore = 0;

		bool running = true;
		bool buttons[4] = {};

		float dt = 0.0f;

		while (running)
		{
			auto startTime = std::chrono::high_resolution_clock::now();


			// XỬ LÍ SỰ KIỆN
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					running = false;
				}
				else if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						running = false;
					}
					else if (event.key.keysym.sym == SDLK_w)
					{
						buttons[Buttons::PaddleOneUp] = true;
					}
					else if (event.key.keysym.sym == SDLK_s)
					{
						buttons[Buttons::PaddleOneDown] = true;
					}
					else if (event.key.keysym.sym == SDLK_UP)
					{
						buttons[Buttons::PaddleTwoUp] = true;
					}
					else if (event.key.keysym.sym == SDLK_DOWN)
					{
						buttons[Buttons::PaddleTwoDown] = true;
					}


				}
				else if (event.type == SDL_KEYUP)
				{
					if (event.key.keysym.sym == SDLK_w)
					{
						buttons[Buttons::PaddleOneUp] = false;
					}
					else if (event.key.keysym.sym == SDLK_s)
					{
						buttons[Buttons::PaddleOneDown] = false;
					}
					else if (event.key.keysym.sym == SDLK_UP)
					{
						buttons[Buttons::PaddleTwoUp] = false;
					}
					else if (event.key.keysym.sym == SDLK_DOWN)
					{
						buttons[Buttons::PaddleTwoDown] = false;
					}
				}
			}



			if (buttons[Buttons::PaddleOneUp])
			{
				paddleOne.velocity.y = -PADDLE_SPEED;
			}
			else if (buttons[Buttons::PaddleOneDown])
			{
				paddleOne.velocity.y = PADDLE_SPEED;
			}
			else
			{
				paddleOne.velocity.y = 0.0f;
			}

			if (buttons[Buttons::PaddleTwoUp])
			{
				paddleTwo.velocity.y = -PADDLE_SPEED;
			}
			else if (buttons[Buttons::PaddleTwoDown])
			{
				paddleTwo.velocity.y = PADDLE_SPEED;
			}
			else
			{
				paddleTwo.velocity.y = 0.0f;
			}


			// Kiểm tra vị trí paddles
			paddleOne.Update(dt);
			paddleTwo.Update(dt);


			// Kiểm tra vị trí Ball
			ball.Update(dt);


			// Kiểm tra và xử lí va chạm
			if (Contact contact = CheckPaddleCollision(ball, paddleOne);
				contact.type != CollisionType::None)
			{
				ball.CollideWithPaddle(contact);


			}
			else if (contact = CheckPaddleCollision(ball, paddleTwo);
				contact.type != CollisionType::None)
			{
				ball.CollideWithPaddle(contact);


			}
			else if (contact = CheckWallCollision(ball);
				contact.type != CollisionType::None)
			{
				ball.CollideWithWall(contact);

				if (contact.type == CollisionType::Left)
				{
					++playerTwoScore;

					playerTwoScoreText.SetScore(playerTwoScore);
				}
				else if (contact.type == CollisionType::Right)
				{
					++playerOneScore;

					playerOneScoreText.SetScore(playerOneScore);
				}
			}



			// Xóa màn hình về đen
			SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
			SDL_RenderClear(renderer);


			// Đặt màu vẽ là màu trắng
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);


			// Vẽ lưới
			for (int y = 0; y < WINDOW_HEIGHT; ++y)
			{
				if (y % 5)
				{
					SDL_RenderDrawPoint(renderer, WINDOW_WIDTH / 2, y);
				}
			}


			// Vẽ Ball
			ball.Draw(renderer);


			// Vẽ Paddles
			paddleOne.Draw(renderer);
			paddleTwo.Draw(renderer);


			// Hiển thị điểm
			playerOneScoreText.Draw();
			playerTwoScoreText.Draw();


			// Present the backbuffer
			SDL_RenderPresent(renderer);


			// Tính thời gian khung hình
			auto stopTime = std::chrono::high_resolution_clock::now();
			dt = std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
		}
	}


	//Dọn dẹp và thoát
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(scoreFont);
	TTF_Quit();
	SDL_Quit();

	return 0;
}

