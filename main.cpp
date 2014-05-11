//	シューティングゲームサンプル
//
//	１年生の夏休み前までに覚えておきたい命令だけでつくられています。
//	わからない命令は調べたり、先生に聞いたりして、
//	夏休み前に理解できるようにしましょう！
//
//	Code by Masao Watanabe

//	DXライブラリを使用するためにヘッダーファイルをインクルードする。
#include "DxLib.h"

//	------------------------------------------------------------------------------------------------------------------------
//	デファイン定義
//	------------------------------------------------------------------------------------------------------------------------

#define OBJECT_PLAYER	1			//	プレイヤーを示すID
#define OBJECT_ENEMY	2			//	敵を示すID

#define STATE_NONE		-1			//	オブジェクトは存在していない
#define STATE_LIVE		1			//	オブジェクトは存在中
#define STATE_HIT		2			//	オブジェクトはぶつかっている

//	プレイヤー情報用データ
#define PLAYER_START_X	(SCREEN_WIDTH / 2)				//	プレイヤーの初期X位置
#define PLAYER_START_Y	(SCREEN_HEIGHT - PLAYER_HEIGHT)	//	プレイヤーの初期Y位置
#define PLAYER_WIDTH	32								//	プレイヤーの横幅
#define PLAYER_HEIGHT	32								//	プレイヤーの縦幅
#define PLAYER_COLOR	0xFFFFFF						//	プレイヤーの色（16進数で記入されています）
#define PLAYER_SHOT_WIDTH	16							//	プレイヤーのショットの横幅
#define PLAYER_SHOT_HEIGHT	16							//	プレイヤーのショットの縦幅
#define PLAYER_SHOT_SPEED	8							//	プレイヤーのショットのスピード
#define PLAYER_MOVESPEED	4							//	プレイヤーの移動量
#define PLAYERSHOT_MAX 10

//	敵情報用データ
#define ENEMY_START_X	0								//	敵の初期X位置
#define ENEMY_START_Y	0								//	敵の初期Y位置
#define ENEMY_WIDTH		32								//	敵の横幅
#define ENEMY_HEIGHT	32								//	敵の縦幅
#define ENEMY_COLOR		0x00FF00						//	敵の色（16進数で記入されています）
#define ENEMY_SHOT_WIDTH	16							//	敵のショットの横幅
#define ENEMY_SHOT_HEIGHT	16							//	敵のショットの縦幅
#define ENEMY_MOVESPEED		2							//	敵の移動量
#define ENEMY_SHOT_SPEED	8							//	敵のショットのスピード
#define ENEMY_SHOT_MIN_TIME	100							//	敵がショットを打つまでの最低間隔
#define ENEMY_SHOT_MAX_TIME	400							//	敵がショットを打つまでの最大間隔
#define ENEMYCOUNT 3

//	背景情報
#define BG_COLOR 0x000000		//	背景の色

#define SCREEN_WIDTH 640			//	スクリーンの横幅
#define SCREEN_HEIGHT 480			//	スクリーンの縦幅

//	------------------------------------------------------------------------------------------------------------------------
//	構造体定義
//	------------------------------------------------------------------------------------------------------------------------

//	オブジェクト構造体。プレイヤー、敵、ショットの元となる構造体
struct OBJECT
{
	int x;			//	X座標
	int y;			//	Y座標
	int width;		//	横幅
	int height;		//	縦幅
	int color;		//	色
	int state;		//	状態を表す変数(-1...存在していない, 1...生きてる, 2...ぶつかった)
};

//	------------------------------------------------------------------------------------------------------------------------
//	グローバル変数定義
//	------------------------------------------------------------------------------------------------------------------------


OBJECT Player;			//	プレイヤー情報
OBJECT Enemy[ENEMYCOUNT];			//	敵情報
OBJECT PlayerShot[PLAYERSHOT_MAX];		//	プレイヤーのショット情報
OBJECT EnemyShot;		//	敵のショット情

int enemyMoveDirc = 1;	//	敵の移動方向
int enemyShotTime = 0;	//	敵が弾をうつタイミング
int PlayerGraph = 0;    // 自機の画像
int EnemyGraph = 0;     //敵機の画像
int ShotSE;             //弾の音

void InitGame(void);
void GameMain(void);

//	--------------------------------------------------------------------
//	メイン関数定義
//	--------------------------------------------------------------------
int WINAPI WinMain(
			HINSTANCE hInstance,      // 現在のインスタンスのハンドル
			HINSTANCE hPrevInstance,  // 以前のインスタンスのハンドル
			LPSTR lpCmdLine,          // コマンドライン
			int nCmdShow              // 表示状態
)
{
	ChangeWindowMode(TRUE);

	//	DXLIBの初期化処理
	DxLib_Init();

	//	ゲーム初期化
	InitGame();

	//	ゲームループ
	//	以下の処理をエスケープキーが押されるまでかプレイヤーが生きている間繰り返す

	while( ProcessMessage() == 0 && CheckHitKey( KEY_INPUT_ESCAPE ) == 0 && Player.state == STATE_LIVE)
	{
		GameMain();
	}

	//	DXLIBの終了処理
	DxLib_End();

	//	アプリケーションの終了
	return 0;
}



//	--------------------------------------------------------------------
//	プレイヤー情報の初期化関数
//	--------------------------------------------------------------------
void InitPlayer()
{
	int value = PLAYER_START_Y;
	PlayerGraph = LoadGraph("player.png");


	//	プレイヤーデータに初期値を入れる
	Player.x = PLAYER_START_X;
	Player.y = PLAYER_START_Y;
	Player.width = PLAYER_WIDTH;
	Player.height = PLAYER_HEIGHT;
	Player.color = PLAYER_COLOR;
	Player.state = STATE_LIVE;

	//	プレイヤーショットデータに初期値を入れる
	//	ショットは存在しない状態からゲームが開始されるので-1で初期化する
	//	座標データはショットを打つまでわからないのでまだ入れない
	for (int i = 0; i < PLAYERSHOT_MAX; i++)
	{
		PlayerShot[i].width = PLAYER_SHOT_WIDTH;
		PlayerShot[i].height = PLAYER_SHOT_HEIGHT;
		PlayerShot[i].color = PLAYER_COLOR;
		PlayerShot[i].state = STATE_NONE;
	}
	

	return;
}

//	--------------------------------------------------------------------
//	敵がショットをうつまでのタイミングを計算する
//	--------------------------------------------------------------------
void CalcEnemyShotTiming()
{
	//	ショットを打つまでの間隔を決める
	//	ENEMY_SHOT_MIN_TIMEからENEMY_SHOT_MAX_TIMEまでの間の乱数をつくる
	enemyShotTime = GetRand(ENEMY_SHOT_MAX_TIME - ENEMY_SHOT_MIN_TIME) + ENEMY_SHOT_MIN_TIME;
}

//	--------------------------------------------------------------------
//	敵情報の初期化関数
//	--------------------------------------------------------------------
void InitEnemy()
{
	EnemyGraph = LoadGraph("enemy.png");

	//	敵データの初期値を入れる
	for (int i = 0; i < ENEMYCOUNT; i++){
		Enemy[i].x = GetRand(SCREEN_WIDTH - ENEMY_WIDTH);	//	毎回ランダムな場所に出現させる
		Enemy[i].y = ENEMY_START_Y;
		Enemy[i].width = ENEMY_WIDTH;
		Enemy[i].height = ENEMY_HEIGHT;
		Enemy[i].color = ENEMY_COLOR;
		Enemy[i].state = STATE_LIVE;
	}

	//	敵ショットデータに初期値を入れる
	//	ショットは存在しない状態からゲームが開始されるので-1で初期化する
	//	座標データはショットを打つまでわからないのでまだ入れない
	EnemyShot.width = ENEMY_SHOT_WIDTH;
	EnemyShot.height = ENEMY_SHOT_HEIGHT;
	EnemyShot.color = ENEMY_COLOR;
	EnemyShot.state = STATE_NONE;

	//	敵の初期移動方向を計算
	//	より広くあいている方向へ移動する
	for (int i = 0; i < ENEMYCOUNT; i++){

		if (Enemy[i].x < SCREEN_WIDTH / 2)
		{
			enemyMoveDirc = 1;
		}
		else
		{
			enemyMoveDirc = -1;
		}

		CalcEnemyShotTiming();
	}
	return;
}

void Resporn(int i)
{
	Enemy[i].x = GetRand(SCREEN_WIDTH - ENEMY_WIDTH);	//	毎回ランダムな場所に出現させる
	Enemy[i].y = ENEMY_START_Y;
	Enemy[i].state = STATE_LIVE;

}

//	--------------------------------------------------------------------
//	ゲーム情報を初期化する関数
//	--------------------------------------------------------------------
void InitGame()
{

	//	プレイヤー情報の初期化
	InitPlayer();
	//	敵情報の初期化
	InitEnemy();
	//  弾の音をロードする
	ShotSE = LoadSoundMem("spo_boxing_panchi03.mp3");


	return;
}

//	--------------------------------------------------------------------
//	オブジェクト描画を行う関数
//	--------------------------------------------------------------------
void DrawObject(OBJECT target)
{
	//	オブジェクトが存在しないときは描画しなくてよい
	if (target.state == STATE_LIVE)
	{


		//	オブジェクト描画
		DrawBox(target.x,			//	X1座標
			target.y,			//	Y1座標
			target.x + target.width - 1,	//	X2座標　この引数は幅を指定するのではなく座標をしているので-1している。
			target.y + target.height - 1,	//	Y2座標　この引数は幅を指定するのではなく座標をしているので-1している。
			target.color,		//	色
			TRUE);				//	塗りつぶしモード(TRUEで塗りつぶし、FALSE)
	}
	return;
}

//	--------------------------------------------------------------------
//	背景を塗りつぶす関数
//	--------------------------------------------------------------------
void DrawBG()
{
	RECT screenRect;	//	描画エリアを格納する変数

	//	描画エリアを取得する
	GetDrawArea(&screenRect);

	//	背景を塗りつぶす
	DrawBox(screenRect.left,		//	X1座標
			screenRect.top,			//	Y1座標
			screenRect.right - 1,	//	X2座標　この引数は幅を指定するのではなく座標をしているので-1している。
			screenRect.bottom - 1,	//	Y2座標　この引数は幅を指定するのではなく座標をしているので-1している。
			BG_COLOR,				//	塗りつぶし色
			TRUE);					//	塗りつぶしモード(TRUEで塗りつぶし、FALSE)

	return;
}


//	--------------------------------------------------------------------
//	objAとobjBが衝突しているかを調べる
//	引数：OBJECT objA,objB...衝突判定したいオブジェクト
//	戻り値：1...衝突している 0...衝突していない
//	--------------------------------------------------------------------
int IsHit(OBJECT objA, OBJECT objB)
{
	int x0 = objA.x;
	int y0 = objA.y;
	int x1 = objA.x + objA.width - 1;
	int y1 = objA.y + objA.height - 1;

	int x2 = objB.x;
	int y2 = objB.y;
	int x3 = objB.x + objB.width - 1;
	int y3 = objB.y + objB.height - 1;

	//	二つの矩形が何かしらの形で重なっているか？
	if (x0 < x3 && x2 < x1 && y0 < y3 && y2 < y1)
	{
		int z = 0;
		z++;
		//	重なっていた
		return 1;
	}


	//	重なってなかった
	return 0;
}
int count = 0;
	
//	--------------------------------------------------------------------
//	弾を生成する関数
//	引数：int shooterID...弾をうつのはプレイヤーか敵なのかの情報
//	引数：OBJECT shooter...弾をうったオブジェクト情報
//	--------------------------------------------------------------------
void CreateShot(int shooterID, OBJECT shooter)
{
	//	自分の弾か敵の弾なのかで処理を変更
	switch(shooterID)
	{
	case OBJECT_PLAYER:
		
		count++;

		if (count == 1){

			for (int i = 0; i < PLAYERSHOT_MAX; i++)
			{
				if (PlayerShot[i].state != STATE_LIVE)
				{
					PlayerShot[i].state = STATE_LIVE;
					PlayerShot[i].x = shooter.x + shooter.width / 2 - PlayerShot[i].width / 2;	//	プレイヤーの真ん中から発射
					PlayerShot[i].y = shooter.y - PlayerShot[i].height;
					//弾の音
					//	PlaySoundMem(ShotSE, DX_PLAYTYPE_BACK);
					break;
				}
			}
			count = 0;
		}
		break;
	case OBJECT_ENEMY:
		EnemyShot.state = STATE_LIVE;
		EnemyShot.x = shooter.x + shooter.width / 2 - EnemyShot.width / 2;	//	敵の真ん中から発射
		EnemyShot.y = shooter.y + shooter.height;
		//弾の音
		PlaySoundMem(ShotSE, DX_PLAYTYPE_BACK); 

		break;
	}

	

}

//	--------------------------------------------------------------------
//	プレイヤーの移動・ショットの発射処理を行う
//	--------------------------------------------------------------------
void ControllPlayer()
{
	//	右カーソルが押されているかどうかをチェック
	if (CheckHitKey(KEY_INPUT_RIGHT) == 1)
	{
		Player.x += PLAYER_MOVESPEED;
	}
	//	左カーソルが押されているかどうかをチェック
	if (CheckHitKey(KEY_INPUT_LEFT) == 1)
	{
		Player.x -= PLAYER_MOVESPEED;
	}


	//　上カーソルが押されているかどうかをチェック
	if (CheckHitKey(KEY_INPUT_UP) == 1)
	{
		Player.y -= PLAYER_MOVESPEED;
	}

	if (CheckHitKey(KEY_INPUT_DOWN) == 1)
	{
		Player.y += PLAYER_MOVESPEED;
	}




	//	ショット発射チェック
	if (CheckHitKey(KEY_INPUT_SPACE) == 1)
	{
		//	弾を生成
		CreateShot(OBJECT_PLAYER, Player);
	}
}

//	--------------------------------------------------------------------
//	プレイヤーのショットの移動関数
//	--------------------------------------------------------------------
void ControllPlayerShot()
{
	for (int i = 0; i < PLAYERSHOT_MAX; i++){
		//	弾が無いか敵にヒットしているなら移動処理はせずに終了
		if (PlayerShot[i].state == STATE_LIVE)
		{

			//	弾の移動
			PlayerShot[i].y -= PLAYER_SHOT_SPEED;

			//	弾は敵にヒットしている？
			if (IsHit(PlayerShot[i], Enemy[i]) == TRUE)
			{
				//	弾を消す
				PlayerShot[i].state = STATE_NONE;

				//	敵をまた出す
				Resporn(i);
				return;
			}

			//	弾は画面外に消えた？
			if (PlayerShot[i].y < 0)
			{
				PlayerShot[i].state = STATE_NONE;
			}
		}
	}
	return;
}

//	--------------------------------------------------------------------
//	敵のショットの移動関数
//	--------------------------------------------------------------------
void ControllEnemyShot()
{
	//	弾が無いかプレイヤーにヒットしているなら移動処理はせずに終了
	if (EnemyShot.state != STATE_LIVE) return;

	//	弾の移動
	EnemyShot.y += ENEMY_SHOT_SPEED;

	//	弾はプレイヤーにヒットしている？
	if (IsHit(EnemyShot, Player) == TRUE)
	{
		//	弾を消す
		EnemyShot.state = STATE_NONE;
		//	プレイヤーを死亡状態にする
		Player.state = STATE_HIT;
		return;
	}

	//	弾は画面外に消えた？
	if (EnemyShot.y > SCREEN_HEIGHT)
	{
		EnemyShot.state = STATE_NONE;
	}

	return;
}


//	--------------------------------------------------------------------
//	敵の移動・ショットの発射処理を行う
//	--------------------------------------------------------------------
void ControllEnemy()
{
	//	端っこまで来ていたら移動方向を反転
	for (int i = 0; i < ENEMYCOUNT; i++){

		switch (enemyMoveDirc)
		{
		case -1:
			if (Enemy[i].x <= 0)
			{
				enemyMoveDirc = 1;
			}
			break;
		case 1:
			if (Enemy[i].x >= SCREEN_WIDTH - ENEMY_WIDTH)
			{
				enemyMoveDirc = -1;
			}
			break;
		}

		//	移動
		Enemy[i].x += ENEMY_MOVESPEED * enemyMoveDirc;

		//	ショット発射チェック
		enemyShotTime--;
		if (enemyShotTime == 0)
		{
			//	弾を現在出していないのなら弾を出す


			if (EnemyShot.state == STATE_NONE)
			{
				//	弾を生成
				CreateShot(OBJECT_ENEMY, Enemy[i]);
			}
			//	次に弾をうつ間隔を決める
			CalcEnemyShotTiming();
		}
	}
}



//	--------------------------------------------------------------------
//	プレイヤー、敵、弾の移動、あたり判定処理を呼ぶ
//	--------------------------------------------------------------------
void ControllGame()
{
	//	プレイヤーの移動判定
	ControllPlayer();

	//	プレイヤーのショットの移動
	ControllPlayerShot();

	//	敵の移動処理
	ControllEnemy();

	//	敵の弾の移動処理
	ControllEnemyShot();

	return;
}

//	--------------------------------------------------------------------
//	プレイヤー、敵、弾、背景の描画処理を呼ぶ
//	--------------------------------------------------------------------
void DrawGame()
{
	// 画面を初期化
	ClearDrawScreen();

	// 描画先画面を裏にする
	SetDrawScreen(DX_SCREEN_BACK);

	//	背景描画
	DrawBG();

	//	プレイヤーの描画処理
	// DrawObject(Player);
	DrawGraph(Player.x - Player.width / 2, Player.y - Player.height / 2, PlayerGraph, TRUE);
	//	プレイヤーの弾描画
	for (int i = 0; i < PLAYERSHOT_MAX; i++){
		DrawObject(PlayerShot[i]);
	}
	//	敵の描画
	//DrawObject(Enemy);

	for (int i = 0; i < ENEMYCOUNT; i++){

	DrawRotaGraph(Enemy[i].x + Enemy[i].width / 2, Enemy[i].y + Enemy[i].height / 2, 1, DX_PI / 180 * 180, EnemyGraph, TRUE);
	//	敵の弾描画
	DrawObject(EnemyShot);
}

	//	裏画面に描いたものを表画面に反映する
	ScreenFlip();

	return;
}


//	--------------------------------------------------------------------
//	様々なゲーム内処理を呼ぶ関数
//	--------------------------------------------------------------------
void GameMain()
{
	//	コントロール
	ControllGame();
	//	描画
	DrawGame();

	return;
}

