#include "qtproj.h"

///////////////////////////////////////////////////////////////////////
// QGameField
///////////////////////////////////////////////////////////////////////
//*							  col[i]  row[j]
void QGameField::setFieldSize(int _x, int _y)
{
	m_xMax = _x;
	m_yMax = _y;
}

void QGameField::setFieldSize(int _xy)
{
	m_xMax = m_yMax = _xy;
}

void QGameField::setCenterPt(QPoint _pt)
{
	m_ptCenter = _pt;
	m_ptLeft = m_ptRight = m_ptUp = m_ptDown = m_ptCenter;
	// not the best practice, but that's how operator= works ... so why not :)
}

bool QGameField::goLeft()
{	
	m_canGoLeft = (bool)m_ptLeft.x(); //  > 0 (LBorder)
	if(m_canGoLeft)
	{
		m_ptLeft.setX(m_ptLeft.x() - 1);
	}
	return m_canGoLeft;
}

bool QGameField::goRight()
{
	m_canGoRight = (m_ptRight.x()+1 < m_xMax); //  < m_xMax (RBorder)
	if (m_canGoRight)
	{
		m_ptRight.setX(m_ptRight.x() + 1);
	}
	return m_canGoRight;
}

bool QGameField::goUp()
{
	m_canGoUp = (bool)m_ptUp.y(); //   > 0 (TopBorder)
	if (m_canGoUp)
	{
		m_ptUp.setY(m_ptUp.y() - 1);
	}
	return m_canGoUp;
}

bool QGameField::goDown()
{
	m_canGoDown = (m_ptDown.y()+1 < m_yMax); //   < m_yMax (BotomBorder)
	if (m_canGoDown)
	{
		m_ptDown.setY(m_ptDown.y() + 1);
	}
	return m_canGoDown;
}

bool QGameField::tryGoAllSidesOneStep()
{
	// true - if the next step still can be performed in any direction
	// false - if the next step can NOT be performed anymore we riched all 4 borders
	return (m_canGoLeft || m_canGoRight || m_canGoUp || m_canGoDown);
}

void QGameField::refreshAllSides()
{
	m_canGoLeft = m_canGoRight = m_canGoUp = m_canGoDown = true;
}

/*bool*/ void QGameField::goAllSidesOneStep()
{
	// true - if the step in any direction successeded
	// false - if the step in any direction failed
	// do it clock-wise
	goUp();
	goRight();
	goDown();
	goLeft();
}

int QGameField::getFieldSize()
{
	int szField = m_xMax;
	// just to make sure we work with NxN matrix
	// IF Magic hapened and 1. matrix is not NxN and 2. user exidently called this function
	if (m_xMax != m_yMax)
	{
		szField = 0;
	}

	return szField;
}

int QGameField::getFieldWidth()
{
	return m_xMax;
}

int QGameField::getFieldHeight()
{
	return m_yMax;
}

int QGameField::getIndxFromPoint(QPoint _pt)
{
	return ( _pt.x() * getFieldSize() + _pt.y() );
}


//*/

///////////////////////////////////////////////////////////////////////
// QMyLabel
///////////////////////////////////////////////////////////////////////

QMyLabel::QMyLabel(const QString& text, QWidget * parent)
	:QLabel(parent), m_pLocker(0), m_pTrigger(0), isAnimProcessing(false)
{
	connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));

	createObject(text);
	QLabel::QLabel(text, parent);
}

QMyLabel::~QMyLabel()
{
	if (m_pLocker)  { SAFE_DELETE(m_pLocker); }
	if (m_pTrigger) { SAFE_DELETE(m_pTrigger); }
}

void QMyLabel::slotClicked()
{
	//qDebug() << "Clicked";
}

void QMyLabel::mousePressEvent(QMouseEvent* event)
{
	bool b = this->isAnimationOn();
	if(!b) // если анимация происходит, то обходим обработчик нажатия мыши
	  emit clicked();		   // если анимания НЕ происходит, значит сигнал нажатия мыши обаботать НУЖНО
}

void QMyLabel::createObject(const QString& objName)
{
	if (objName == "Locker")
	{
		m_pLocker = new Locker();
	}
	else if (objName == "Trigger")
	{
		m_pTrigger = new Trigger();
	}
}

///////////////////////////////////////////////////////////////////////
// QMainWindow
///////////////////////////////////////////////////////////////////////

QtProj::QtProj(QWidget *parent)
	: QMainWindow(parent)
	, m_pngLock(PIC_LOCKER_LOCK)
	, m_pngUnLock(PIC_LOCKER_UNLOCK)
	, m_pngTrigger(PIC_TRIGGER)
	, m_gameField(new QGameField(FOUR))
	, m_labEnterName(new QLabel(this))
	, m_editEnterName(new QTextEdit(this))
	, m_labFieldSizeOptions(new QLabel(this))
	, m_cboxFieldSizeOptions(new QComboBox(this))
	, m_btnTest(new QPushButton("TEST", this))
	, m_btnStartGame(new QPushButton("START THE GAME", this))
	, m_btnReStartGame(new QPushButton("RESTART THE GAME", this))
	, m_btnStepBack(new QPushButton("<- Step BACK", this))
	, m_btnStepForw(new QPushButton("Step FORWARD ->", this))
	, m_listwRecords(new QListWidget(this))
	, m_widgetLayout(new QVBoxLayout(this))
	, m_labGameTime(new QLabel(this))
	, m_counter(0)
	, m_isGameStarted(false)
	, m_labPrevToGoBack(0)
	, m_labCurrentToGoForward(0)
{
	testFun();

	loadRecordsFromFile();

	//ui.setupUi(this);
	initUI();
}

QtProj::~QtProj()
{
	try
	{
		saveRecordsToFile();
	}
	catch (...)
	{
		showMessage("Error: Inside Destructor safely handled");
	}
}


void QtProj::initUI()
{
	// Установим размер самого окна
	m_qszMainWondowSize.setWidth(1280);
	m_qszMainWondowSize.setHeight(720);
	this->setMinimumSize(m_qszMainWondowSize);

	////////////////////////////////////////////////////////////
	//// TEST BUTTON
	m_btnTest->move(TEN,TEN);
	m_btnTest->adjustSize();
	//m_btnTest->setVisible(false);
	connect(m_btnTest, SIGNAL(clicked()), this, SLOT(testSlot()));

	////////////////////////////////////////////////////////////
	//// Editbox to enter Name
	m_labEnterName->setText("Player name");
	m_labEnterName->adjustSize();
	m_editEnterName->move(((m_qszMainWondowSize.width() / 2 / FOUR) * (FOUR - 1) + 1) + 100, TEN);

	QRect r = m_labEnterName->fontMetrics().boundingRect(m_labEnterName->text());
	int textWidth = r.width();
	QPoint p = m_editEnterName->pos();
	m_labEnterName->move(p.x() - textWidth - TEN, p.y() + TEN);

	connect(m_editEnterName, SIGNAL(textChanged()), this, SLOT(handlePlayerName()));

	////////////////////////////////////////////////////////////
	//// Combobox GameField Options
	m_labFieldSizeOptions->setText("Choose Gamefield size"); //("Выберите размер игровоо поля");
	m_labFieldSizeOptions->adjustSize();

	m_cboxFieldSizeOptions->addItem("4 x 4");
	m_cboxFieldSizeOptions->addItem("10 x 10");
	m_cboxFieldSizeOptions->move(((m_qszMainWondowSize.width() / 2 / FOUR) * (FOUR - 3) + 1) + 100, TEN); //  m_qszMainWondowSize.height()

	r = m_labFieldSizeOptions->fontMetrics().boundingRect(m_labFieldSizeOptions->text());
	textWidth = r.width();
	p = m_cboxFieldSizeOptions->pos();
	m_labFieldSizeOptions->move(p.x() - textWidth - TEN, p.y() + TEN);

	connect(m_cboxFieldSizeOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFieldSize()));

	////////////////////////////////////////////////////////////
	//// Buttons - 1 step Back / Resume / Start the Game
	m_btnStartGame->setEnabled(false);
	m_btnReStartGame->setEnabled(false);
	m_btnStepBack->setEnabled(false);
	m_btnStepForw->setEnabled(false);

	m_btnStepBack->move(((m_qszMainWondowSize.width()    / 2 / FOUR) * 0        + 1) + 100, TEN * FOUR + TEN);
	m_btnReStartGame->move(((m_qszMainWondowSize.width() / 2 / FOUR) * (FOUR-3) + 1) + 100, TEN * FOUR + TEN);
	m_btnStartGame->move(((m_qszMainWondowSize.width()   / 2 / FOUR) * (FOUR-2) + 1) + 100, TEN * FOUR + TEN);
	m_btnStepForw->move(((m_qszMainWondowSize.width()    / 2 / FOUR) * (FOUR-1) + 1) + 100, TEN * FOUR + TEN);

	m_btnStartGame->adjustSize();
	m_btnReStartGame->adjustSize();
	m_btnStepForw->adjustSize();

	connect( m_btnStepBack,   SIGNAL(clicked()), this, SLOT(goStepBack()) );
	connect(m_btnStartGame,   SIGNAL(clicked()), this, SLOT(startGame()) );
	connect(m_btnReStartGame, SIGNAL(clicked()), this, SLOT(restartGame()));
	connect( m_btnStepForw,   SIGNAL(clicked()), this, SLOT(goStepForw()) );

	////////////////////////////////////////////////////////////
	//// Records Window - 10 best games
	m_listwRecords->setMinimumSize(300, 300);
	m_listwRecords->move(((m_qszMainWondowSize.width() / 2 / FOUR) * FOUR + 1) + 100, TEN * TEN);

	////////////////////////////////////////////////////////////
	//// Create 4 or 10 Animated Widgets for the game
	QRect rect(100, 100, 800, 800);
	m_widgetLayout->setGeometry(rect);
	
#if USE_TEST_FUCTIONS
	winnerQListWidgets(m_gameField->getFieldSize()); // this fuction is for tests
#else
	shufleQListWidgets(m_gameField->getFieldSize());
#endif	
	//disableQListWidgets(m_gameField->getFieldSize());



	////////////////////////////////////////////////////////////
	//// Create Timer
	m_timerGameTime = new QTimer(this);
	connect(m_timerGameTime, SIGNAL(timeout()), this, SLOT(showGameTime()));

	QFont serifFont("Times", 20, QFont::Bold);
	m_labGameTime->setFont(serifFont);
	m_labGameTime->setText("00 : 00 : 00");
	m_labGameTime->move(((m_qszMainWondowSize.width() / 2 / FOUR) * FOUR + 1) + 100, FOUR * TEN * TEN + TEN);
	QSize qsz(300, 200); // [w , h]
	m_labGameTime->setFixedSize(qsz);
	
	////////////////////////////////////////////////////////////
	//// After everything is created, first check for locked / unlocked Lockers
	continueGameLogic();
}

///////////////////////////////////////////////////////////////////////
// USER DEFINED FUNCTIONS

void QtProj::loadRecordsFromFile()
{
	QFile file(RECORDS_FILE_NAME);
	if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
	{
		return; // could not open for some reaseon
	}

	QTextStream strmFromFile(&file);
	while (!strmFromFile.atEnd())
	{
		//QString line = "John|1234";
		QString line = strmFromFile.readLine();

		QStringList personRecord = line.split("|");
		QString playerName	= personRecord.value(PLAYER_NAME);
		int playerTime		= personRecord.value(PLAYER_SCORE).toInt();

		m_mapRecords.insert(playerTime, playerName);
	}

	file.close();


	if (m_mapRecords.size() > TEN) // erase all what is above 10 elements
	{
		auto it = m_mapRecords.begin() + TEN;
		auto end = m_mapRecords.end();
		while (it != end)
		{
			it = m_mapRecords.erase(it);
		}
	}

	loadMapToListWidget();
}

void QtProj::saveRecordsToFile()
{
	if (m_mapRecords.size()) // fill RecordsList
	{
		QFile file(RECORDS_FILE_NAME);
		if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
		{
			return; // could not open for some reaseon
		}
		QTextStream strmToFile(&file);


		short i = 1;
		RecordsIt it(m_mapRecords);
		it.toFront();
		int keyPrev = 0;
		while (it.hasNext())
		{
			it.next();
			int key = it.key();
			//it.value();

			QList<QString> values = m_mapRecords.values(key);
			int ilen = values.size();
			
			if((keyPrev != key) && (ilen>0))
			{
				do
				{
					ilen--;
					QString value = values.at(ilen);

					// Name|Score
					strmToFile << value + "|" + QString::number(key) << endl;
					i++;
	
				} while (ilen != 0);
			}

			keyPrev = key;
		} // WHILE

		file.close();
	}
	else
	{
		showMessage("ERROR: No Records have been stored !");
	}
}

void QtProj::refreshListRecords()
{
	m_listwRecords->clear();
	loadMapToListWidget();
}

void QtProj::loadMapToListWidget()
{
	if (m_mapRecords.size()) // fill RecordsList
	{
		short i = 1;
		RecordsIt it(m_mapRecords);
		//it.toBack();
		it.toFront();
		int keyNext = 0; //keyPrev = 0;
		while (it.hasNext()) //(it.hasPrevious())
		{
			//it.previous();
			it.next();
			int key = it.key();
			//it.value();

			QList<QString> values = m_mapRecords.values(key);
			int ilen = values.size();
			
			if((keyNext != key) && (ilen>0))
			{
				do
				{
					ilen--;
					QString value = values.at(ilen);

					m_listwRecords->addItem(QString::number(i) + ". " + QString::number(key) + "  - " + value);
					i++;
	
				} while (ilen != 0);
			}

			keyNext = key;
		}
	}
	else
	{
		showMessage("ERROR: Unable to load Records !");
	}
}

void QtProj::addRecordToListAndFile()
{
	// Add record to:
	// 1. MMap Container
	// 2. List on the Interface
	// 3. To the File

	QString playerName = m_editEnterName->toPlainText();
	int playerTime = m_time.elapsed();

	Records it = m_mapRecords.find(playerTime);
	while (it != m_mapRecords.end() && it.key() == playerTime)
	{
		// totaly identical record from the same user found
		return; //it.value();
	}

	m_mapRecords.insert(playerTime, playerName);

	int numRecords = 1;
	auto it_rem = m_mapRecords.begin();
	auto end_rem = m_mapRecords.end();
	while(it_rem != end_rem)
	{
		if (numRecords > 10)
		{
			it_rem = m_mapRecords.erase(it_rem); // m_mapRecords.erase(it_rem++);
		}
		else
		{
			++it_rem;
		}
		numRecords++;
	}

	refreshListRecords();
	saveRecordsToFile();
}

void QtProj::showQListWidgets(int _numWidgets)
{
	if (m_vecLockers.size() > 0 && m_vecTriggers.size() > 0)
	{
		for (int i = 0; i < _numWidgets; i++)
		{
			m_vecLockers[i]->setVisible(true);
			for (int j = 0; j < _numWidgets; j++)
			{
				m_vecTriggers[i][j]->setVisible(true);
				m_vecTriggers[i][j]->runAnimation();
			}
		}
	}
}

void QtProj::hideQListWidgets(int _numWidgets)
{
	if(m_vecLockers.size() > 0 && m_vecTriggers.size() > 0)
	{
		for (int i = 0; i < _numWidgets; i++)
		{
			m_vecLockers[i]->setVisible(false);   // с этой строчкой внутри  recreateQListWidgets скрываются предыдущие замки 4 и поверх рисуются и отображаются новые 10
			for (int j = 0; j < _numWidgets; j++) // без неё замки рисуются одни поверх других
			{
				m_vecTriggers[i][j]->setVisible(false);
				m_vecTriggers[i][j]->stopAnimation();
			}
		}
	}
}


void QtProj::enableQListWidgets(int _numWidgets)
{
	if (m_vecLockers.size() > 0 && m_vecTriggers.size() > 0)
	{
		for (int i = 0; i < _numWidgets; i++)
		{
			m_vecLockers[i]->setEnabled(true);
			for (int j = 0; j < _numWidgets; j++)
			{
				m_vecTriggers[i][j]->stopAnimation();
			}
		}
	}
}

void QtProj::disableQListWidgets(int _numWidgets)
{
	if (m_vecLockers.size() > 0 && m_vecTriggers.size() > 0)
	{
		for (int i = 0; i < _numWidgets; i++)
		{
			m_vecLockers[i]->setEnabled(false);
			for (int j = 0; j < _numWidgets; j++)
			{
				m_vecTriggers[i][j]->runAnimation();
			}
		}
	}
}

// if (!this->isAnimationOn()) // false => emit, true => exit
// emit clicked();			   // если мы делаем emit => то начнётся прокручивание ручки, а если обошли emit, то ручка не повернётся, даже если на неё будем нажимать
// isAnimProcessing = false по умолчанию
//void runAnimation() { isAnimProcessing = true; }	 //  
//void stopAnimation() { isAnimProcessing = false; } // 
//bool isAnimationOn() { return isAnimProcessing; }	 // 

void QtProj::shufleQListWidgets(int _curNumWidgets)
{
	recreateQListWidgets(0, _curNumWidgets);
}

void QtProj::recreateQListWidgets(int _prevNumWidgets, int _curNumWidgets)
{
	qsrand(QDateTime::currentMSecsSinceEpoch() / 1000); // init for qrand()

	hideQListWidgets(_prevNumWidgets);

	// clear before recreating the new List
	if (!m_vecLockers.isEmpty())
	{
		m_vecLockers.clear();
	}
	if (!m_vecTriggers.isEmpty())
	{
		m_vecTriggers.clear();
	}

	for (int i = 0, x = 0, y = 0; i < _curNumWidgets; i++) // i, x, col
	{
		m_vecLockers.push_back(new QMyLabel("Locker", this));

		x = ((m_qszMainWondowSize.width()  / 2 / _curNumWidgets) * i) + 100;
		y = ((m_qszMainWondowSize.height() / 2 / _curNumWidgets) * 0) + 90;
		m_vecLockers[i]->move(x, y);

		//m_vecLockers[i]->setText(QString::number(i));
		//m_vecLockers[i]->adjustSize();
		m_vecLockers[i]->setPixmap(m_pngLock);
		m_vecLockers[i]->setFixedSize(m_pngLock.size());

		// Save all the info we will need for the rotation and game logic
		//m_vecLockers[i]->m_pLocker->setLocker(i, false);
		m_vecLockers[i]->m_pLocker->setLocker(i, 0, false);

		QVector<QMyLabel*> tmpTriggers;
		for (int j = 0, indx = 0 ; j < _curNumWidgets; j++) // j, y, row
		{
			indx = (i*_curNumWidgets) + j;
			tmpTriggers.push_back(new QMyLabel("Trigger", this));

			x = ((m_qszMainWondowSize.width()  / 2   / _curNumWidgets) * i) + 100;
			y = ((m_qszMainWondowSize.height() / 1.5 / _curNumWidgets) * j) + 200 ;
			tmpTriggers[j]->move(x, y);

			//tmpTriggers[j]->setText(QString::number(i) + QString::number(j));
			//tmpTriggers[j]->adjustSize();
			//*
			bool bRand = (bool)(qrand() % 2); // true _ horizontal, false | vertical
			int degree = bRand ? DEGREES_90 : 0;
			tmpTriggers[j]->setPixmap(rotatePic(degree, m_pngTrigger));
			tmpTriggers[j]->setFixedSize(m_pngTrigger.size());

			// Save all the info we will need for the rotation and game logic
			//tmpTriggers[j]->m_pTrigger->setTrigger(indx, degree);
			tmpTriggers[j]->m_pTrigger->setTrigger(i, j, degree);
			//*/

			connect(tmpTriggers[j], SIGNAL(clicked()), this, SLOT(handleRotationTriggerClick()));
		}

		m_vecTriggers.push_back(tmpTriggers);
	}

	//update();
	//repaint();
	showQListWidgets(_curNumWidgets);
}


void QtProj::winnerQListWidgets(int _curNumWidgets)
{
	qsrand(QDateTime::currentMSecsSinceEpoch() / 1000); // init for qrand()

	hideQListWidgets(_curNumWidgets);

	// clear before recreating the new List
	if (!m_vecLockers.isEmpty())
	{
		m_vecLockers.clear();
	}
	if (!m_vecTriggers.isEmpty())
	{
		m_vecTriggers.clear();
	}

	for (int i = 0, x = 0, y = 0; i < _curNumWidgets; i++) // i, x, col
	{
		m_vecLockers.push_back(new QMyLabel("Locker", this));

		x = ((m_qszMainWondowSize.width() / 2 / _curNumWidgets) * i) + 100;
		y = ((m_qszMainWondowSize.height() / 2 / _curNumWidgets) * 0) + 90;
		m_vecLockers[i]->move(x, y);

		m_vecLockers[i]->setPixmap(m_pngLock);
		m_vecLockers[i]->setFixedSize(m_pngLock.size());
		m_vecLockers[i]->m_pLocker->setLocker(i, 0, false);

		QVector<QMyLabel*> tmpTriggers;
		for (int j = 0, indx = 0; j < _curNumWidgets; j++) // j, y, row
		{
			indx = (i * _curNumWidgets) + j;
			tmpTriggers.push_back(new QMyLabel("Trigger", this));

			x = ((m_qszMainWondowSize.width() / 2 / _curNumWidgets) * i) + 100;
			y = ((m_qszMainWondowSize.height() / 1.5 / _curNumWidgets) * j) + 200;
			tmpTriggers[j]->move(x, y);

			bool bRand = !(i == 0 || j == 0); // all vertical exept i=0/j=0 // true _ horizontal, false | vertical
			int degree = bRand ? DEGREES_90 : 0;
			tmpTriggers[j]->setPixmap(rotatePic(degree, m_pngTrigger));
			tmpTriggers[j]->setFixedSize(m_pngTrigger.size());

			// Save all the info we will need for the rotation and game logic
			tmpTriggers[j]->m_pTrigger->setTrigger(i, j, degree);

			connect(tmpTriggers[j], SIGNAL(clicked()), this, SLOT(handleRotationTriggerClick()));
		}

		m_vecTriggers.push_back(tmpTriggers);
	}

	showQListWidgets(_curNumWidgets);
}


void QtProj::findHorizontalTriggersCol(const QVector<QVector<QMyLabel*>>& _vecTriggers, const QVector<QMyLabel*>& _vecLockers, const int _numWidgets)
{
	if (m_vecLockers.size() > 0 && m_vecTriggers.size() > 0)
	{
		for (int i = 0, ilen = _vecTriggers.size(), horizontalTriggers = 0; i < ilen; i++, horizontalTriggers = 0)
		{
			for (int j = 0, jlen = _vecTriggers[0].size(); j < jlen; j++)
			{
				bool b = m_vecTriggers[i][j]->m_pTrigger->m_state;
				if (b)
				{
					horizontalTriggers++;
				}
			}

			if (horizontalTriggers == _numWidgets)
			{	// open if were closed
				m_vecLockers[i]->m_pLocker->m_state = true; // opened
				performLockOpen(_vecLockers[i]);
			}
			else
			{	// close if were open
				m_vecLockers[i]->m_pLocker->m_state = false; // closed
				performLockClose(_vecLockers[i]);
			}
		}
	}
}

bool QtProj::checkOpenedLocks(const QVector<QMyLabel*>& _vecLockers)
{
	bool areAllLocksOpen = false;
	foreach(QMyLabel* locker, _vecLockers)
	{
		areAllLocksOpen = locker->m_pLocker->m_state;
		if (!locker->m_pLocker->m_state)
		{
			break; // if there will be even one False, we break with that false and return it
		}
	}
	// if there are only TRUEs we will exit foreach with the last true and return it
	return areAllLocksOpen;
}

void QtProj::performLockOpen(QMyLabel* _lab)
{
	//QMyLabel* lab = qobject_cast<QMyLabel*>(sender());
	if (_lab)
	{
		_lab->setPixmap(rotatePic(0, m_pngUnLock));
	}
}

void QtProj::performLockClose(QMyLabel* _lab)
{
	//QMyLabel* lab = qobject_cast<QMyLabel*>(sender());
	if (_lab)
	{
		_lab->setPixmap(rotatePic(0, m_pngLock));
	}
}

void QtProj::setRefreshPic(QMyLabel* _lab, QPixmap& _pic)
{
	_lab->setVisible(false);
	_lab->setPixmap(_pic);
	_lab->setVisible(true);
}


void QtProj::performFastDraw(QMyLabel* _lab)
{
	//QMyLabel* lab = qobject_cast<QMyLabel*>(sender());
	if (_lab)
	{
		// before rotation disable all Lables to ensure Player is unable to do anything during the animation
		disableQListWidgets(m_gameField->getFieldSize());
		m_counter = _lab->m_pTrigger->m_degree + DEGREES_90;
		_lab->setPixmap(rotatePic(m_counter, m_pngTrigger));
		_lab->m_pTrigger->setTrigger(m_counter);
	}
}

void QtProj::performRotation(QMyLabel* _lab)
{
#if USE_TEST_FUCTIONS
	// Fast Rotation for Tests
	performFastDraw(_lab);
#else
	//* 
	//QMyLabel* lab = qobject_cast<QMyLabel*>(sender());
	if (_lab)
	{
		// before rotation disable all Lables to ensure Player is unable to do anything during the animation
		disableQListWidgets(m_gameField->getFieldSize());

		QElapsedTimer timer;
		timer.start();
		
		m_counter  = _lab->m_pTrigger->m_degree;
		int counterEnd = _lab->m_pTrigger->m_degree + DEGREES_90;
		while (true)
		{
			QApplication::processEvents();
			if (timer.elapsed() % MOVEMENT_SPEED == 0)
			{
				_lab->setPixmap(rotatePic(m_counter, m_pngTrigger));
				m_counter++;
			}

			// 1. n1=0;  n1+90 = 90; n2 goes up from n1=0  0/90, 1/90...   90/90=1
			// 2. n1=90; n1+90 =180; n2 goes up from n1=90  90/180, 91.. 180/180=1
			if(m_counter / counterEnd) // 90 degree rotation is complete
			{
				_lab->m_pTrigger->setTrigger(m_counter);
				//_lab->m_pTrigger->m_degree = m_counter;
				//_lab->m_pTrigger->m_state = !(_lab->m_pTrigger->m_degree % DEGREES_180 == 0);
				//if (_lab->m_pTrigger->m_state)
				//	_lab->m_pTrigger->m_degree = 0;

				break;
			}
		}
	}
	//*/
#endif

}


QPixmap QtProj::rotatePic(int _counter, QPixmap& _pic)
{
	QPixmap rotate(_pic.size());

	QPainter p(&rotate);
	p.setRenderHint(QPainter::Antialiasing);
	p.setRenderHint(QPainter::SmoothPixmapTransform);
	p.setRenderHint(QPainter::HighQualityAntialiasing);
	p.translate(rotate.size().width() / 2, rotate.size().height() / 2);
	p.rotate(_counter); // counter = number of degrees of the angle
	p.translate(-rotate.size().width() / 2, -rotate.size().height() / 2);

	p.drawPixmap(0, 0, _pic);
	p.end();

	return rotate;
}


void QtProj::showMessage(QString _msg)
{
	QMessageBox msgBox;
	msgBox.setText(_msg);
	msgBox.exec(); //Display message
}


///////////////////////////////////////////////////////////////////////
// SLOTS

void QtProj::handlePlayerName()
{
	QString playerName = m_editEnterName->toPlainText();
	bool isEmptyName = playerName.isEmpty();

	m_btnStartGame->setEnabled(!isEmptyName);
	m_btnReStartGame->setEnabled(!isEmptyName);
}

void QtProj::stopGame()
{
	disableQListWidgets(m_gameField->getFieldSize());
	stopGameTime();
	addRecordToListAndFile();
}

void QtProj::startGame()
{
	enableQListWidgets(m_gameField->getFieldSize());
	startGameTime();
}

void QtProj::restartGame()
{
	//enableQListWidgets(m_gameField->getFieldSize());
	m_labGameTime->setText("00 : 00 : 00");
	shufleQListWidgets(m_gameField->getFieldSize());
	continueGameLogic();
	startGameTime();
}


void QtProj::goStepBack()
{
	m_btnStepBack->setEnabled(false);
	m_btnStepForw->setEnabled(true);
	handleStepBackRotateTrigger();
}

void QtProj::goStepForw()
{
	m_btnStepForw->setEnabled(false);
	m_btnStepBack->setEnabled(true);
	handleStepForwardRotateTrigger();
}


void QtProj::changeFieldSize()
{
	int prevFieldSize = m_gameField->getFieldSize();
	switch (m_cboxFieldSizeOptions->currentIndex())
	{
	case (FIELD_SZ_4):
		m_gameField->setFieldSize(FOUR);
		break;
	case (FIELD_SZ_10):
		m_gameField->setFieldSize(TEN);
		break;
	}

	if (prevFieldSize != m_gameField->getFieldSize())
	{
		recreateQListWidgets(prevFieldSize, m_gameField->getFieldSize());
		continueGameLogic();
	}
}


void QtProj::handleLockerClick()
{
	QMyLabel* lab = qobject_cast<QMyLabel*>(sender());
	if (lab)
	{
		//int indx = lab->m_pLocker->m_indx;
	}
}



void QtProj::performRotationСross(QMyLabel* _lab)
{
	if (_lab)
	{
		m_gameField->setCenterPt(QPoint(_lab->m_pTrigger->m_x, _lab->m_pTrigger->m_y));

		for (int i = 1, ilen = m_gameField->getFieldSize(); i < ilen; i++)
		{
			//QThread( performRotation(m_vecTriggers.at(indxL)) );
			if (!m_gameField->tryGoAllSidesOneStep())
			{
				m_gameField->refreshAllSides();
				break;
			}

			// do the rotations clock-wise
			if (m_gameField->goUp())		 { performRotation(m_vecTriggers[m_gameField->getUp().x()]	 [m_gameField->getUp().y()]);	 }
			else if (m_gameField->goRight()) { performRotation(m_vecTriggers[m_gameField->getRight().x()] [m_gameField->getRight().y()]); }
			else if (m_gameField->goDown())	 { performRotation(m_vecTriggers[m_gameField->getDown().x()]	 [m_gameField->getDown().y()]);	 }
			else if (m_gameField->goLeft())	 { performRotation(m_vecTriggers[m_gameField->getLeft().x()]	 [m_gameField->getLeft().y()]);	 }
			//*/
		}

		m_gameField->refreshAllSides();
		continueGameLogic();
	}
}


void QtProj::handleRotationTriggerClick()
{
	m_btnStepBack->setEnabled(true);
	m_btnStepForw->setEnabled(false);

	// perform rotation of clicked Triggers
	if (m_labCurrentToGoForward == 0) // to be able to resume after the very first step after the actual Start of the game
	{
		m_labCurrentToGoForward = qobject_cast<QMyLabel*>(sender());
	}

	m_labPrevToGoBack = m_labCurrentToGoForward;
	m_labCurrentToGoForward = qobject_cast<QMyLabel*>(sender());
	
	QMyLabel* lab = m_labCurrentToGoForward;
	performRotation(lab);

	// perform rotation of other Cross (+) standing Neighbours Triggers
	performRotationСross(lab);
}


void QtProj::handleStepForwardRotateTrigger()
{
	// perform rotation of clicked Triggers
	QMyLabel* lab = m_labCurrentToGoForward;
	performRotation(lab);

	// perform rotation of other Cross (+) standing Neighbours Triggers
	performRotationСross(lab);
}

void QtProj::handleStepBackRotateTrigger()
{
	// perform rotation of clicked Triggers
	QMyLabel* lab = m_labPrevToGoBack;
	performRotation(lab);

	// perform rotation of other Cross (+) standing Neighbours Triggers
	performRotationСross(lab);
}


void QtProj::continueGameLogic()
{
	// check if any of the Triggers in any Column stand ALL Horizontal
	// if so, then open the Lock over such Column of horizontal Triggers
	findHorizontalTriggersCol(m_vecTriggers, m_vecLockers, m_gameField->getFieldSize());

	// check the Locks vector. If all of them are Open Then:
	// 0. Disable ALL Triggers and Lockers 1. Stop Timer 2. Save Players Result to container +RecordsList 
	// 3. ReEnable the buttons to refresh and start a new game 4. While starting a new game reshufle the Triggers positions
	
	if (checkOpenedLocks(m_vecLockers))
	{
		// GAME HAS ENDED, bacause player solved the puzzle
		stopGame();
	}
	else
	{
		// The Game goes on, so we enable all the Game elements
		if(m_isGameStarted)
		enableQListWidgets(m_gameField->getFieldSize());
	}
}

void QtProj::startGameTime()
{
	m_timerGameTime->start();
	m_time.start();
	m_isGameStarted = true;

	m_editEnterName->setEnabled(false);
	// Enabled after the End of the Game (*)
	m_cboxFieldSizeOptions->setEnabled(false); 
}

void QtProj::stopGameTime()
{
	m_timerGameTime->stop();
	m_isGameStarted = false;

	m_editEnterName->setEnabled(true);
	// Enabled after the End of the Game (*)
	m_cboxFieldSizeOptions->setEnabled(true); 
}

void QtProj::showGameTime()
{
	if (m_isGameStarted)
	{
		int elapsed = m_time.elapsed();
		if (elapsed % 20 == 0)
		{
			QTime elapsed = m_time.fromMSecsSinceStartOfDay(m_time.elapsed());
			QString timeText = elapsed.toString("hh : mm : ss");
			m_labGameTime->setText(timeText);
		}
	}
}


///////////////////////////////////////////////////////////////////////
// TESTS
/*
void QtProj::Tell_App(QString _words)
{
	showMessage("App");
	showMessage(_words);

	emit testSlot();
}

void QtProj::testSlot()
{
	//disableQListWidgets(m_gameField->getFieldSize());
	//stopGameTime();
}

void QtProj::testFun()
{
	//startGameTime();
}
//*/


