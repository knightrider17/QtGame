#ifndef QTPROJ_H
#define QTPROJ_H

// QT
#include <QtWidgets/QMainWindow>

#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMessageBox>

#include <QFile>
#include <QTextStream>

#include <QPainter>
#include <QPaintEvent>

#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QTimer>
#include <QDateTime>

#include <QThread>


// STL
//#include <algorithm>

//////////////////////
#include "ui_qtproj.h"
//#include "myLabel.h"


typedef QMapIterator<int, QString> RecordsIt;
typedef QMultiMap<int, QString>::iterator Records;

#define USE_TEST_FUCTIONS 0


#define SAFE_DELETE(x) delete(x); x=0;

#define FOUR 4
#define TEN 10

#define DEGREES_90  90
#define DEGREES_180 180

#define MOVEMENT_SPEED 40 // bigger -> Speed down // lesser -> Speed up

#define PIC_LOCKER_UNLOCK	"://QtProj//Resources//lock-S-unlock.png"
#define PIC_LOCKER_LOCK		"://QtProj//Resources//lock-S.png"
#define PIC_TRIGGER			"://QtProj//Resources//trigger-S.png"

#define RECORDS_FILE_NAME  "scores.txt"

///////////////////////////////////////////////////////////////////////
// QGameField
///////////////////////////////////////////////////////////////////////
/*	 c_1 c_2 . c_N
	--------------> x = col = i
r_1	|
r_1	|
...	|
r_N	|
	v
	y = row = j
*/

//*
class QGameField
{
public:
	QGameField(int _xFieldSz, int _yFieldSz) // N x M Field size
		: m_xMax(_xFieldSz), m_yMax(_yFieldSz)
		, m_canGoLeft(true), m_canGoRight(true)
		, m_canGoUp(true), m_canGoDown(true)
	{}
	QGameField(int _xyFieldSz) // N X N Field size
		: m_xMax(_xyFieldSz), m_yMax(_xyFieldSz)
		, m_canGoLeft(true), m_canGoRight(true)
		, m_canGoUp(true), m_canGoDown(true)
	{}
	//~QGameField() {}
private:
	int m_xMax; // X = Width
	int m_yMax; // Y = Height

	bool m_canGoLeft;
	bool m_canGoRight;
	bool m_canGoUp;
	bool m_canGoDown;

	QPoint m_ptCenter;
	QPoint m_ptLeft;
	QPoint m_ptRight;
	QPoint m_ptUp;
	QPoint m_ptDown;
public:
	void setFieldSize(int _xy);
	void setFieldSize(int _x, int _y);
	void setCenterPt(QPoint _pt);
	
	bool goLeft();
	bool goRight();
	bool goUp();
	bool goDown();
	void goAllSidesOneStep();
	bool tryGoAllSidesOneStep();
	void refreshAllSides();

	int getFieldSize();	// for N X N Field size
	int getFieldWidth();  // for N x M Field size // X = Width
	int getFieldHeight(); // for N x M Field size // Y = Height
	int getIndxFromPoint(QPoint _pt);

	QPoint getCenter()	{ return m_ptCenter; }
	QPoint getLeft()	{ return m_ptLeft;	 }
	QPoint getRight()	{ return m_ptRight;	 }
	QPoint getUp()		{ return m_ptUp;	 }
	QPoint getDown()	{ return m_ptDown;	 }
	
	int getCenterIndx()	{ return getIndxFromPoint(m_ptCenter); }
	int getLeftIndx()	{ return getIndxFromPoint(m_ptLeft  ); }
	int getRightIndx()	{ return getIndxFromPoint(m_ptRight ); }
	int getUpIndx()		{ return getIndxFromPoint(m_ptUp	); }
	int getDownIndx()	{ return getIndxFromPoint(m_ptDown	); }

};
//*/


///////////////////////////////////////////////////////////////////////
// QMyLabel
///////////////////////////////////////////////////////////////////////
class QMyLabel : public QLabel
{
	Q_OBJECT
private:
	bool isAnimProcessing;
public:
	QMyLabel(const QString& text, QWidget* parent = 0);
	~QMyLabel();

	void createObject(const QString& objName);

	void runAnimation()  { isAnimProcessing = true; }
	void stopAnimation() { isAnimProcessing = false; }
	bool isAnimationOn() { return isAnimProcessing; }

	struct Locker
	{
		bool m_state;	// opened / closed
		//int m_indx;	// index = i
		int m_x;		// i, col, x
		int m_y;		// j, row, y
		//Locker() : m_state(false), m_indx(-1) {}
		Locker() : m_state(false), m_x(-1), m_y(-1) {}

		//void setLocker(int _indx, int _state)
		void setLocker(int _x, int _y, int _state)
		{
			//m_indx	= _indx;
			m_x = _x;
			m_y = _y;
			m_state	= _state;
		}
	} *m_pLocker;

	struct Trigger : public Locker
	{
		//bool state;	// true = horizontal / false = vertical
		//int indx;		// index = i*numElem + j
		int m_degree;	// current element Degree = 90 or 0 / 180 grad
		Trigger() : m_degree(0) {}
		
		//void setTrigger(int _indx, int _degree)
		void setTrigger(int _x, int _y, int _degree)
		{
			//m_indx	 = _indx;
			m_x	 = _x;
			m_y	 = _y;
			setTrigger(_degree);
		}
		void setTrigger(int _degree)
		{
			if (_degree % 180 == 0) _degree = 0;
			m_degree = _degree;
			m_state = (bool)((_degree / 90) % 2); // 0/180 = false, 90 = true
		}

	} *m_pTrigger;

signals:
	void clicked();

	public slots:
	void slotClicked();

protected:
	void mousePressEvent(QMouseEvent* event);

};

///////////////////////////////////////////////////////////////////////
// QMainWindow
///////////////////////////////////////////////////////////////////////
class QtProj : public QMainWindow
{
	Q_OBJECT

public:
	QtProj(QWidget *parent = 0);
	~QtProj();

	void Tell_App(QString _words);

private:
	Ui::QtProjClass ui;

	enum FIELD_SIZE
	{
		  FIELD_SZ_4 = 0
		, FIELD_SZ_10
	};

	enum RECORDS_INFO
	{
		  PLAYER_NAME = 0
		, PLAYER_SCORE
	};


// user defined QControls:
	QSize m_qszMainWondowSize;

	QLabel* m_labEnterName;
	QTextEdit* m_editEnterName;

	QLabel* m_labFieldSizeOptions;
	QComboBox* m_cboxFieldSizeOptions;

	QPushButton* m_btnStartGame;
	QPushButton* m_btnReStartGame;
	QPushButton* m_btnStepBack;
	QPushButton* m_btnStepForw;

	QListWidget* m_listwRecords;
	QFile* m_fileRecords;
	QMultiMap<int, QString> m_mapRecords; // QMultiMap because many equal records can have many different players

	QVBoxLayout* m_widgetLayout;

	QVector<QMyLabel*> m_vecLockers;
	QVector<QVector<QMyLabel*>> m_vecTriggers;

	QMyLabel* m_labPrevToGoBack; // by clicking BACK (to the PAST)
	QMyLabel* m_labCurrentToGoForward; // by clicking FORWARD (to the FUTURE that has happened) 

	QLabel* m_labGameTime;
	QTimer* m_timerGameTime;


	QPushButton* m_btnTest;

// user defined variables:
	QPixmap m_pngLock;
	QPixmap m_pngUnLock;
	QPixmap m_pngTrigger;

	QGameField* m_gameField;
	int m_counter;

	QTime m_time;
	bool m_isGameStarted;

// user defined functions:
	void testFun();
	void initUI();

	void refreshListRecords();
	void loadMapToListWidget();
	void loadRecordsFromFile();
	void saveRecordsToFile();
	void addRecordToListAndFile();

	void disableQListWidgets(int _numWidgets);
	void enableQListWidgets(int _numWidgets);
	
	void hideQListWidgets(int _numWidgets);
	void showQListWidgets(int _numWidgets);
	void recreateQListWidgets(int _prevNumWidgets, int _curNumWidgets);
	void shufleQListWidgets(int _curNumWidgets);

	void winnerQListWidgets(int _curNumWidgets); // our cheeting function for tests
	void performFastDraw(QMyLabel* _lab);		 // our faster rotate and drawing function for tests

	void performRotation(QMyLabel* _lab);
	void performRotationÑross(QMyLabel* _lab); // CROSS = Êðåñòîì
	QPixmap rotatePic(int _counter, QPixmap& _pic);
	void setRefreshPic(QMyLabel* _lab, QPixmap& _pic);
	void findHorizontalTriggersCol(const QVector<QVector<QMyLabel*>>& _vecTriggers, const QVector<QMyLabel*>& _vecLockers, const int _numWidgets);
	void performLockClose(QMyLabel* _lab);
	void performLockOpen(QMyLabel* _lab);
	bool checkOpenedLocks(const QVector<QMyLabel*>& _vecLockers);
	void continueGameLogic();

// user defined get/set functions:
public:
	bool getIsGameStarted()					{ return m_isGameStarted; }
	void setIsGameStarted(bool _isStarted)	{ m_isGameStarted = _isStarted; }

	void showMessage(QString _msg);

// user defined slots:
public slots:
	void testSlot();

	void handlePlayerName();

	void goStepBack();
	void goStepForw();

	void startGame();
	void restartGame();
	void stopGame();

	void changeFieldSize();

	void handleLockerClick();
	void handleRotationTriggerClick();	// handler for the real mouseclick on the pressed Trigger and start rotation process

	void handleStepForwardRotateTrigger(); // Back to the FUTURE by programmaticly press  the next saved pressed Trigger and start rotation process
	void handleStepBackRotateTrigger(); // BACK to the PAST Step by programmaticly resume the previous saved pressed Trigger and start rotation process

	void showGameTime();
	void startGameTime();
	void stopGameTime();
};

#endif // QTPROJ_H
