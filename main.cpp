#include "DxLib.h"
#include <cstdlib>
#include <ctime>
#include <vector>

template <typename T>
class Vector2D
{
	public:
		T x;
		T y;

		Vector2D() : x(0), y(0) {}

		Vector2D(T x, T y)
		{
			this->x = x;
			this->y = y;
		};
};

enum Lane
{
	Up,
	Center,
	Down,
};

struct LanePosition
{
	Lane lane;
	Vector2D<float> position;
};

LanePosition upStart = { Up, Vector2D<float>(50, 140) };
LanePosition centerStart = { Center, Vector2D<float>(50, 240) };
LanePosition downStart = { Down, Vector2D<float>(50, 340) };

LanePosition upEnd = { Up, Vector2D<float>(650, 140) };
LanePosition centerEnd = { Center, Vector2D<float>(650, 240) };
LanePosition downEnd = { Down, Vector2D<float>(650, 340) };


class ObjectRender
{
	int playerSize = 20;
	int enemySize = 20;

public:
	void PlayerDrawInLane(LanePosition lanePosition)
	{
		auto x = lanePosition.position.x;
		auto y = lanePosition.position.y;

		int color = GetColor(255, 255, 255);
		DrawCircleAA(x, y, playerSize, 32, color, true);
	}

	void EnemyDraw(Vector2D<float> postion)
	{
		auto x = postion.x;
		auto y = postion.y;

		int color = GetColor(255, 0, 0);
		DrawTriangleAA(x - 50, y, x, y + 30, x, y - 30, color, TRUE);
	}

	void EnemyDrawInLane(LanePosition lanePosition)
	{
			auto x = lanePosition.position.x;
			auto y = lanePosition.position.y;

			int color = GetColor(255, 0, 0);
			DrawTriangleAA(x - 50, y, x , y + 30, x , y - 30, color, TRUE);
	}

	void StageDraw()
	{
		int color = GetColor(255, 255, 255);

		// レーン
		for (int i = 0; i < 4; i++)
		{
			DrawLine(0, 95 + (i * 100), 640, 95 + (i * 100), color);
		}
	}
};

class Player
{
	public:
		ObjectRender* objectRender;

		Player(ObjectRender* objectRender)
		{
			this->objectRender = objectRender;
		}

		void DrawPlayer()
		{
			switch (playerPosition)
			{
			case 0:
				objectRender->PlayerDrawInLane(upStart);
				break;
			case 1:
				objectRender->PlayerDrawInLane(centerStart);
				break;
			case 2:
				objectRender->PlayerDrawInLane(downStart);
				break;
			default:
				break;
			}
		}

		void UpdatePlayerPostion()
		{
			switch (playerPosition)
			{
			case 0:
				position = upStart.position;
				break;
			case 1:
				position = centerStart.position;
				break;
			case 2:
				position = downStart.position;
				break;
			default:
				break;
			}
		}

		void Up()
		{
			playerPosition--;

			if (playerPosition < 0) playerPosition = 0;

			UpdatePlayerPostion();
		}

		void Down()
		{
			playerPosition++;

			if (playerPosition > 2) playerPosition = 2;

			UpdatePlayerPostion();
		}

		Vector2D<float> GetPosition()
		{
			return position;
		}

	private:
		// 0 :上 1:中央 2:下
		char playerPosition = 1;
		Vector2D<float> position = centerStart.position;
};

class Enemy
{
	public:
		ObjectRender* objectRender;
		Vector2D<float> position;

		Enemy(ObjectRender* objectRender, LanePosition startLine)
		{
			this->objectRender = objectRender;
			this->startLine = &startLine;
			this->position =  Vector2D<float>(startLine.position.x, startLine.position.y);
		}

		void DrawEnemy()
		{
			objectRender->EnemyDrawInLane(*startLine);
		}

		void DrawEnemy(Vector2D<float> position)
		{
			objectRender->EnemyDraw(position);
		}

	private:
		LanePosition* startLine;
};

class EnemyManager {
	public:
		EnemyManager(ObjectRender* objectRender)
		{
			objectRender = objectRender;	
		};


		void SpawnEnemy() {
			srand(static_cast<unsigned int>(time(nullptr)));

			auto rand = GetRand(3);
			auto randomPosition = rand;	
			LanePosition spawnLane;

			switch (randomPosition) {
			case 0:
				spawnLane = upEnd;
				break;
			case 1:
				spawnLane = centerEnd;
				break;
			case 2:
				spawnLane = downEnd;
				break;
			default:
				break;
			}

			Enemy enemy = Enemy(objectRender, spawnLane);
			enemies.push_back(enemy);
		}

		void Update() {
			spawnTimer--;
			if (spawnTimer < 0) {
				SpawnEnemy();
				spawnTimer = spawnRate;

				// 間隔を短くする
				if (spawnRate > 25) {
					spawnRate -= 3;
				}
			}

			// 敵を動かす
			for (size_t i = 0; i < enemies.size(); i++) {
				enemies[i].position.x -= 5;

				if (enemies[i].position.x < 0) {
					// 画面外に出た敵を削除
					enemies.erase(enemies.begin() + i);
					i--; 
				}
				else {
					enemies[i].DrawEnemy(enemies[i].position);
				}
			}
		}

		std::vector<Enemy> GetEnemies() {
			return enemies;
		}

	private:
		ObjectRender* objectRender;
		std::vector<Enemy> enemies;
		int rand = GetRand(3);
		float spawnTimer = 60;
		float spawnRate = 60;
};

bool isGameEnd()
{
	// Windows システムからくる情報を処理する
	if (ProcessMessage() == -1) return true;
	// ＥＳＣキーが押されたらループから抜ける
	if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) return true;
}

// WinMain関数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	bool isBeforeFrameInput = false;

	// オブジェクトの描画を行うクラス
	ObjectRender objectRender;

	// プレイヤーの描画を行うクラス
	Player player = Player(&objectRender);

	// 敵の描画を行うクラス
	EnemyManager enemyManager = EnemyManager(&objectRender);

	// ＤＸライブラリ初期化処理
	if (DxLib_Init() == -1) return -1;

	objectRender = ObjectRender();


	//タイトル画面
	DrawString(250, 200, "タイトル画面", GetColor(255, 255, 255));
	DrawString(250, 250, "PRESS ANYKEY", GetColor(255, 255, 255));

	WaitKey();


	// 描画先画面を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	while (true)
	{
		ClearDrawScreen();
		objectRender.StageDraw();

		// インプット
		if (!isBeforeFrameInput)
		{
			if (CheckHitKey(KEY_INPUT_UP)) player.Up();
			if (CheckHitKey(KEY_INPUT_DOWN)) player.Down();
		}

		// 敵の更新
		enemyManager.Update();

		// プレイヤーを描く
		player.DrawPlayer();

		// 前のフレームで入力があったかどうか
		if (CheckHitKey(KEY_INPUT_UP) == 1 || CheckHitKey(KEY_INPUT_DOWN) == 1)
		{
			isBeforeFrameInput = true;
		}
		else
		{
			isBeforeFrameInput = false;
		}


		// 当たり判定
		bool isHit = false;
		for (size_t i = 0; i < enemyManager.GetEnemies().size(); i++)
		{
			if (enemyManager.GetEnemies()[i].position.y == player.GetPosition().y)
			{
				auto distance = abs(enemyManager.GetEnemies()[i].position.x - player.GetPosition().x);
				if (distance < 20)
				{
					isHit = true;
					break;
				}
			}
		}


		ScreenFlip();

		WaitTimer(20);

		if (isGameEnd()) break;

		if (isHit) break;
	}

	ClearDrawScreen();

	// ゲームオーバー
	DrawString(250, 200, "GAME OVER", GetColor(255, 0, 0));
	DrawString(250, 250, "PRESS ANYKEY", GetColor(255, 255, 255));

	ScreenFlip();

	WaitKey();


	// ＤＸライブラリ使用の終了処理
	DxLib_End();

	// ソフトの終了
	return 0;
}
