#ifndef LOWERTHIRDSWITCHERDOCKWIDGET_H
#define LOWERTHIRDSWITCHERDOCKWIDGET_H

#include <string>
#include <iostream>
#include <list>
#include <util/base.h>
#include <util/platform.h>
#include <util/config-file.h>
#include <vector>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <future>
#include <unistd.h>

#include <Qt>
#include <QMainWindow>
#include <QDockWidget>
#include <QEvent>
#include <QPushButton>
#include <QObject>
#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QDropEvent>
#include <QModelIndexList>
#include <QIcon>
#include <QPixmap>
#include <QString>

#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>

#include <plugin-support.h>
#include "ui_lowerthirdswitcher.h"

#define CONFIG "config.json"

class LowerthirdswitcherDockWidget : public QWidget {
	Q_OBJECT
public:
	explicit LowerthirdswitcherDockWidget(QWidget *parent = nullptr);
	~LowerthirdswitcherDockWidget();

	int nextButtonHotkeyId = -1;

	struct lowerthirditem {
		QString mainText;
		QString secondaryText;
	};

private:
	enum SourceType { TEXT_SOURCE = 1, GROUP_SOURCE = 2, SCENE_SOURCE = 3 };

	Ui::LowerThirdSwitcher *ui;

	static void OBSSourceCreated(void *param, calldata_t *calldata);
	static void OBSSourceDeleted(void *param, calldata_t *calldata);
	static void OBSSourceRenamed(void *param, calldata_t *calldata);

	static void OBSFrontendEventHandler(enum obs_frontend_event event,
					    void *private_data);

	static int CheckSourceType(obs_source_t *source);
	static void LoadSavedSettings(Ui::LowerThirdSwitcher *ui,
				      LowerthirdswitcherDockWidget *context);

private slots:
	void ConnectObsSignalHandlers();
	void DisonnectObsSignalHandlers();
	void ConnectUISignalHandlers();

	void SaveSettings();

	void RegisterHotkeys(LowerthirdswitcherDockWidget *context);
	void UnregisterHotkeys();

	void sceneChanged(QString scene);
	void groupSourceChanged(QString newText);
	void mainTextSourceChanged(QString newText);
	void secondaryTextSourceChanged(QString newText);
	void displayTimeValueChanged(int newTime);

	void nextItem();

	void addNewItemClicked();
	void setActiveItemClicked();
	void deleteItemClicked();
	void editItemClicked(QListWidgetItem *listWidgetItem);

	void mainTextEdited(QString newText);
	void secondaryTextEdited(QString newText);

	void LoadItemsToList();

	void setActiveItem(int i);
	void setCurrentSceneCollection();
	const char *getCurrentSceneCollection();
};

#endif // LOWERTHIRDSWITCHERDOCKWIDGET_H
