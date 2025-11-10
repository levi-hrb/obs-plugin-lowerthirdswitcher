#include "lowerthirdswitcher-widget.hpp"

QString selectedScene;
QString selectedGroupFolderSource;
QString selectedMainTextSource;
QString selectedSecondaryTextSource;
int displayDuration;
std::vector<LowerthirdswitcherDockWidget::lowerthirditem> lowerthirditems;
int activeItem = 0;

const char *currentSceneCollection = "none";

LowerthirdswitcherDockWidget::LowerthirdswitcherDockWidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::LowerThirdSwitcher)
{
	ui->setupUi(this);

	setMinimumSize(300, 380);

	setCurrentSceneCollection();

	ConnectUISignalHandlers();
	ConnectObsSignalHandlers();

	obs_frontend_add_event_callback(OBSFrontendEventHandler, this);

	RegisterHotkeys(this);
}

LowerthirdswitcherDockWidget::~LowerthirdswitcherDockWidget()
{
	signal_handler_disconnect(obs_get_signal_handler(), "source_destroy",
				  OBSSourceDeleted, ui);
	SaveSettings();
	UnregisterHotkeys();
}

void LowerthirdswitcherDockWidget::ConnectUISignalHandlers()
{

	QObject::connect(ui->sceneDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(sceneChanged(QString)));
	QObject::connect(ui->groupSourceDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(groupSourceChanged(QString)));
	QObject::connect(ui->mainTextSourceDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(mainTextSourceChanged(QString)));
	QObject::connect(ui->secondaryTextSourceDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(secondaryTextSourceChanged(QString)));
	QObject::connect(ui->displayTimeSpinBox, SIGNAL(valueChanged(int)),
			 SLOT(displayTimeValueChanged(int)));

	QObject::connect(ui->nextButton, SIGNAL(clicked()), SLOT(nextItem()));

	QObject::connect(ui->addNewItemButton, SIGNAL(clicked()),
			 SLOT(addNewItemClicked()));
	QObject::connect(ui->activeItemButton, SIGNAL(clicked()),
			 SLOT(setActiveItemClicked()));
	QObject::connect(ui->deleteItemButton, SIGNAL(clicked()),
			 SLOT(deleteItemClicked()));
	QObject::connect(ui->itemsListWidget,
			 SIGNAL(itemClicked(QListWidgetItem *)),
			 SLOT(editItemClicked(QListWidgetItem *)));

	QObject::connect(ui->mainTextLineEdit, SIGNAL(textEdited(QString)),
			 SLOT(mainTextEdited(QString)));
	QObject::connect(ui->secondaryTextLineEdit, SIGNAL(textEdited(QString)),
			 SLOT(secondaryTextEdited(QString)));

	QObject::connect(ui->itemsListWidget,
			 SIGNAL(itemPressed(QListWidgetItem *)),
			 SLOT(itemsListWidgetItemPressed(QListWidgetItem *)));
	QObject::connect(ui->itemsListWidget, SIGNAL(dropEvent(QDropEvent *)),
			 SLOT(listWidgetDropEvent(QDropEvent *)));
}

void LowerthirdswitcherDockWidget::sceneChanged(QString newText)
{
	selectedScene = newText;
}

void LowerthirdswitcherDockWidget::groupSourceChanged(QString newText)
{
	selectedGroupFolderSource = newText;
}

void LowerthirdswitcherDockWidget::mainTextSourceChanged(QString newText)
{
	selectedMainTextSource = newText;
}

void LowerthirdswitcherDockWidget::secondaryTextSourceChanged(QString newText)
{
	selectedSecondaryTextSource = newText;
}

void LowerthirdswitcherDockWidget::displayTimeValueChanged(int newTime)
{
	displayDuration = newTime;
}

void LowerthirdswitcherDockWidget::nextItem()
{
	int runningActiveItem =
		activeItem; // redeclared, because activeItem will be changed while other thread still use runningActiveItem
	obs_source_t *groupFolderSource = obs_get_source_by_name(
		selectedGroupFolderSource.toStdString().c_str());
	if (groupFolderSource != NULL) {
		obs_scene_t *scene = obs_get_scene_by_name(
			selectedScene.toStdString().c_str());
		obs_sceneitem_t *sceneItem = obs_scene_find_source(
			scene, selectedGroupFolderSource.toStdString().c_str());
		obs_sceneitem_set_visible(sceneItem, true);

		obs_source_t *mainTextSource = obs_get_source_by_name(
			selectedMainTextSource.toStdString().c_str());
		if (mainTextSource != NULL &&
		    (int)(lowerthirditems.size()) > runningActiveItem) {
			obs_data_t *sourceSettings =
				obs_source_get_settings(mainTextSource);
			obs_data_set_string(sourceSettings, "text",
					    lowerthirditems[runningActiveItem]
						    .mainText.toStdString()
						    .c_str());
			obs_source_update(mainTextSource, sourceSettings);
			obs_data_release(sourceSettings);
		}
		obs_source_release(mainTextSource);

		obs_source_t *secondaryTextSource = obs_get_source_by_name(
			selectedSecondaryTextSource.toStdString().c_str());
		if (secondaryTextSource != NULL &&
		    (int)(lowerthirditems.size()) > runningActiveItem) {
			obs_data_t *sourceSettings =
				obs_source_get_settings(secondaryTextSource);
			obs_data_set_string(sourceSettings, "text",
					    lowerthirditems[runningActiveItem]
						    .secondaryText.toStdString()
						    .c_str());
			obs_source_update(secondaryTextSource, sourceSettings);
			obs_data_release(sourceSettings);
		}
		obs_source_release(secondaryTextSource);

		std::thread t{[=]() {
			try {
				std::this_thread::sleep_for(
					std::chrono::milliseconds(
						displayDuration));

				if (scene) {
					obs_sceneitem_t *sceneItem =
						obs_scene_find_source(
							scene,
							selectedGroupFolderSource
								.toStdString()
								.c_str());
					if (sceneItem)
						obs_sceneitem_set_visible(
							sceneItem, false);
				}
			} catch (int i) {
			}
		}};
		t.detach();

		std::thread tflash{[=]() {
			int flashDuration = 500;
			int duration =
				((abs(displayDuration) + flashDuration - 1) /
				 flashDuration) *
				((flashDuration * displayDuration) /
				 abs(displayDuration)); // round up to multiple of flashDuration

			// it is important to check with [if (ui->itemsListWidget->item(runningActiveItem))] to ensure, that the item has not been changed in the meantime
			for (int i = 0; i < duration / flashDuration / 2; i++) {
				if (ui->itemsListWidget->item(
					    runningActiveItem))
					ui->itemsListWidget
						->item(runningActiveItem)
						->setIcon(QIcon(
							":/red-icon.png"));
				ui->nextButton->setIcon(
					QIcon(":/red-icon.png"));
				std::this_thread::sleep_for(
					std::chrono::milliseconds(
						flashDuration));
				if (ui->itemsListWidget->item(
					    runningActiveItem))
					ui->itemsListWidget
						->item(runningActiveItem)
						->setIcon(QIcon(
							":/red-flash-icon.png"));
				ui->nextButton->setIcon(
					QIcon(":/red-flash-icon.png"));
				std::this_thread::sleep_for(
					std::chrono::milliseconds(
						flashDuration));
			}
			QPixmap px(8, 8);
			px.fill(Qt::transparent);
			if (ui->itemsListWidget->item(runningActiveItem))
				ui->itemsListWidget->item(runningActiveItem)
					->setIcon(QIcon(px));
			ui->nextButton->setIcon(
				QIcon(":/red-green-transition-icon.png"));
			if (ui->itemsListWidget->item(activeItem)) {
				ui->itemsListWidget->item(activeItem)
					->setIcon(QIcon(
						":/green-icon.png")); // in case the item has been manually set back to active
			}
		}};
		tflash.detach();

		obs_source_release(groupFolderSource);
		obs_scene_release(scene);

		// !!! STEP TO NEXT ITEM !!! //

		if ((int)(lowerthirditems.size()) > activeItem + 1) {
			activeItem = activeItem +
				     1; // if new index exists, apply new index
		} else if ((int)(lowerthirditems.size()) > 0) {
			activeItem = 0; // if end is reached, jump back to start
		}
		try {
			QListWidgetItem *item =
				ui->itemsListWidget->item(activeItem);
			if (item)
				item->setIcon(QIcon(":/green-icon.png"));
		} catch (int i) {
		}
	}
}

void LowerthirdswitcherDockWidget::RegisterHotkeys(
	LowerthirdswitcherDockWidget *context)
{
	auto LoadHotkey = [](obs_data_t *s_data, obs_hotkey_id id,
			     const char *name) {
		if ((int)id == -1)
			return;
		OBSDataArrayAutoRelease array =
			obs_data_get_array(s_data, name);

		obs_hotkey_load(id, array);
		obs_data_array_release(array);
	};

	char *file = obs_module_config_path(CONFIG);
	obs_data_t *saved_data = nullptr;
	if (file) {
		saved_data = obs_data_create_from_json_file(file);
		bfree(file);
	}

#define HOTKEY_CALLBACK(pred, method)                                          \
	[](void *incoming_data, obs_hotkey_id, obs_hotkey_t *, bool pressed) { \
		Ui::LowerThirdSwitcher &lowerThirdSwitcherUI =                 \
			*static_cast<Ui::LowerThirdSwitcher *>(incoming_data); \
		if ((pred) && pressed) {                                       \
			method();                                              \
		}                                                              \
	}

	// Register NextButton Hotkey
	context->nextButtonHotkeyId = (int)obs_hotkey_register_frontend(
		"Next_Lower_Third_Hotkey", "Next Lower Third",
		HOTKEY_CALLBACK(true,
				lowerThirdSwitcherUI.nextButton->animateClick),
		ui);
	if (saved_data)
		LoadHotkey(saved_data, context->nextButtonHotkeyId,
			   "Next_Lower_Third_Hotkey");

	obs_data_release(saved_data);
#undef HOTKEY_CALLBACK
}

void LowerthirdswitcherDockWidget::UnregisterHotkeys()
{
	if (nextButtonHotkeyId)
		obs_hotkey_unregister(nextButtonHotkeyId);
}

void LowerthirdswitcherDockWidget::addNewItemClicked()
{
	LowerthirdswitcherDockWidget::lowerthirditem item;

	item.mainText = "New Main Text";
	item.secondaryText = "New Secondary Text";
	if (item.mainText != "" || item.secondaryText != "") {
		lowerthirditems.push_back(item);
		LoadItemsToList();
		ui->itemsListWidget->scrollToBottom();
		ui->itemsListWidget->item(ui->itemsListWidget->count() - 1)
			->setSelected(true);
		ui->itemsListWidget->setCurrentRow(
			ui->itemsListWidget->count() - 1);
		editItemClicked(ui->itemsListWidget->item(
			ui->itemsListWidget->count() - 1));
	}
}

void LowerthirdswitcherDockWidget::setActiveItemClicked()
{
	activeItem = ui->itemsListWidget->currentRow();
	LoadItemsToList();
}

void LowerthirdswitcherDockWidget::deleteItemClicked()
{
	if (ui->itemsListWidget->selectedItems().size() != 0) {
		lowerthirditems.erase(lowerthirditems.begin() +
				      ui->itemsListWidget->currentRow());
		LoadItemsToList();
	}
}

void LowerthirdswitcherDockWidget::editItemClicked(
	QListWidgetItem *listWidgetItem)
{
	int idx = ui->itemsListWidget->row(listWidgetItem);
	LowerthirdswitcherDockWidget::lowerthirditem item =
		lowerthirditems[idx];

	ui->mainTextLineEdit->setText(item.mainText);
	ui->secondaryTextLineEdit->setText(item.secondaryText);
	ui->mainTextLineEdit->setEnabled(true);
	ui->secondaryTextLineEdit->setEnabled(true);
}

void LowerthirdswitcherDockWidget::mainTextEdited(QString newText)
{
	int idx = ui->itemsListWidget->currentRow();
	LowerthirdswitcherDockWidget::lowerthirditem item =
		lowerthirditems[idx];
	item.mainText = newText;
	lowerthirditems[idx] = item;
	int currentCursorPosition = ui->mainTextLineEdit->cursorPosition();
	LoadItemsToList();
	ui->mainTextLineEdit->setCursorPosition(currentCursorPosition);
}

void LowerthirdswitcherDockWidget::secondaryTextEdited(QString newText)
{
	int idx = ui->itemsListWidget->currentRow();
	LowerthirdswitcherDockWidget::lowerthirditem item =
		lowerthirditems[idx];
	item.secondaryText = newText;
	lowerthirditems[idx] = item;
	int currentCursorPosition = ui->secondaryTextLineEdit->cursorPosition();
	LoadItemsToList();
	ui->secondaryTextLineEdit->setCursorPosition(currentCursorPosition);
}

void LowerthirdswitcherDockWidget::LoadItemsToList()
{
	int currentRow = ui->itemsListWidget->currentRow();
	ui->itemsListWidget->clear();

	for (std::vector<LowerthirdswitcherDockWidget::lowerthirditem>::iterator
		     it = lowerthirditems.begin();
	     it != lowerthirditems.end(); ++it) {
		QListWidgetItem *listItem = new QListWidgetItem(it->mainText);
		QPixmap px(8, 8);
		px.fill(Qt::transparent);
		listItem->setIcon(QIcon(px));
		ui->itemsListWidget->addItem(listItem);
	}
	// check that a row was selected in the first place, and that this row has not been deleted
	if (currentRow != -1 &&
	    currentRow <
		    ui->itemsListWidget
			    ->count()) { // set origionally selected item as active
		ui->itemsListWidget->item(currentRow)->setSelected(true);
		ui->itemsListWidget->setCurrentRow(currentRow);
		editItemClicked(ui->itemsListWidget->item(currentRow));
	} else if (ui->itemsListWidget->count() !=
		   0) { // if non selected and list not empty, select last item
		ui->itemsListWidget->item(ui->itemsListWidget->count() - 1)
			->setSelected(true);
		ui->itemsListWidget->setCurrentRow(
			ui->itemsListWidget->count() - 1);
		editItemClicked(ui->itemsListWidget->item(
			ui->itemsListWidget->count() - 1));
	} else { // if no items exist, disable edit box
		ui->mainTextLineEdit->setText("");
		ui->secondaryTextLineEdit->setText("");
		ui->mainTextLineEdit->setEnabled(false);
		ui->secondaryTextLineEdit->setEnabled(false);
	}
	if (ui->itemsListWidget->item(activeItem)) {
		ui->itemsListWidget->item(activeItem)
			->setIcon(QIcon(":/green-icon.png"));
	}
}

void LowerthirdswitcherDockWidget::OBSFrontendEventHandler(
	enum obs_frontend_event event, void *private_data)
{
	LowerthirdswitcherDockWidget *context =
		(LowerthirdswitcherDockWidget *)private_data;
	Ui::LowerThirdSwitcher *ui = context->ui;

	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING: {
		ui->nextButton->setIcon(
			QIcon(":/red-green-transition-icon.png"));
		ui->activeItemButton->setIcon(QIcon(":/green-icon.png"));
		ui->mainTextLineEdit->setEnabled(false);
		ui->secondaryTextLineEdit->setEnabled(false);
		// LowerthirdswitcherDockWidget::ConnectUISignalHandlers(context);
		LoadSavedSettings(ui, context);
	} break;
	case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING: {
		context->setCurrentSceneCollection();
		context->SaveSettings();
		// context->DisonnectObsSignalHandlers();
	} break;
	case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED: {
		context->setCurrentSceneCollection();
		LoadSavedSettings(ui, context);
	} break;
	default:
		break;
	}
}

void LowerthirdswitcherDockWidget::ConnectObsSignalHandlers()
{
	// Source Signals
	signal_handler_connect(obs_get_signal_handler(), "source_create",
			       OBSSourceCreated, ui);
	signal_handler_connect(obs_get_signal_handler(), "source_destroy",
			       OBSSourceDeleted, ui);
	signal_handler_connect(obs_get_signal_handler(), "source_rename",
			       OBSSourceRenamed, ui);
}

void LowerthirdswitcherDockWidget::DisonnectObsSignalHandlers()
{
	// Source Signals
	signal_handler_disconnect(obs_get_signal_handler(), "source_create",
				  OBSSourceCreated, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_destroy",
				  OBSSourceDeleted, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_rename",
				  OBSSourceRenamed, ui);
}

void LowerthirdswitcherDockWidget::OBSSourceCreated(void *param,
						    calldata_t *calldata)
{
	auto ui = static_cast<Ui::LowerThirdSwitcher *>(param);
	obs_source_t *source;
	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	int sourceType = CheckSourceType(source);
	if (!sourceType)
		return;

	const char *name = obs_source_get_name(source);

	if (sourceType == TEXT_SOURCE) {
		ui->mainTextSourceDropdownList->addItem(name);
		ui->secondaryTextSourceDropdownList->addItem(name);
	} else if (sourceType == GROUP_SOURCE) {
		ui->groupSourceDropdownList->addItem(name);
	} else if (sourceType == SCENE_SOURCE) {
		ui->sceneDropdownList->addItem(name);
	}
};

void LowerthirdswitcherDockWidget::OBSSourceDeleted(void *param,
						    calldata_t *calldata)
{
	auto ui = static_cast<Ui::LowerThirdSwitcher *>(param);

	obs_source_t *source;

	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	int sourceType = CheckSourceType(source);
	if (!sourceType)
		return;

	const char *name = obs_source_get_name(source);

	if (sourceType == TEXT_SOURCE) {
		ui->mainTextSourceDropdownList->removeItem(
			ui->mainTextSourceDropdownList->findText(name));
		ui->secondaryTextSourceDropdownList->removeItem(
			ui->secondaryTextSourceDropdownList->findText(name));
	} else if (sourceType == GROUP_SOURCE) {
		ui->groupSourceDropdownList->removeItem(
			ui->groupSourceDropdownList->findText(name));
	} else if (sourceType == SCENE_SOURCE) {
		ui->sceneDropdownList->removeItem(
			ui->sceneDropdownList->findText(name));
	}
};

void LowerthirdswitcherDockWidget::OBSSourceRenamed(void *param,
						    calldata_t *calldata)
{
	auto ui = static_cast<Ui::LowerThirdSwitcher *>(param);

	obs_source_t *source;
	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	int sourceType = CheckSourceType(source);
	if (!sourceType)
		return;

	const char *newName = calldata_string(calldata, "new_name");
	const char *oldName = calldata_string(calldata, "prev_name");

	if (sourceType == TEXT_SOURCE) {
		int idx = ui->mainTextSourceDropdownList->findText(oldName);
		if (idx == -1)
			return;
		ui->mainTextSourceDropdownList->setItemText(idx, newName);
		idx = ui->secondaryTextSourceDropdownList->findText(oldName);
		if (idx == -1)
			return;
		ui->secondaryTextSourceDropdownList->setItemText(idx, newName);
	} else if (sourceType == GROUP_SOURCE) {
		int idx = ui->groupSourceDropdownList->findText(oldName);
		if (idx == -1)
			return;
		ui->groupSourceDropdownList->setItemText(idx, newName);
	} else if (sourceType == SCENE_SOURCE) {
		int idx = ui->sceneDropdownList->findText(oldName);
		if (idx == -1)
			return;
		ui->sceneDropdownList->setItemText(idx, newName);
	}
};

int LowerthirdswitcherDockWidget::CheckSourceType(obs_source_t *source)
{
	const char *source_id = obs_source_get_unversioned_id(source);
	if (strcmp(source_id, "text_ft2_source") == 0 ||
	    strcmp(source_id, "text_gdiplus") == 0 ||
	    strcmp(source_id, "text_pango_source") == 0) {
		return TEXT_SOURCE;
	} else if (strcmp(source_id, "group") == 0) {
		return GROUP_SOURCE;
	} else if (strcmp(source_id, "scene") == 0) {
		return SCENE_SOURCE;
	}
	return 0;
}

void LowerthirdswitcherDockWidget::LoadSavedSettings(
	Ui::LowerThirdSwitcher *ui, LowerthirdswitcherDockWidget *context)
{
	char *file = obs_module_config_path(CONFIG);
	obs_data_t *obsSettingsData = obs_data_create_from_json_file(file);
	bfree(file);

	if (obsSettingsData) {
		obs_data_t *data = obs_data_get_obj(
			obsSettingsData, context->getCurrentSceneCollection());

		// Get Saved Data
		selectedScene = QString::fromUtf8(
			obs_data_get_string(data, "selectedScene"));
		selectedGroupFolderSource = QString::fromUtf8(
			obs_data_get_string(data, "selectedGroupFolderSource"));
		selectedMainTextSource = QString::fromUtf8(
			obs_data_get_string(data, "selectedMainTextSource"));
		selectedSecondaryTextSource = QString::fromUtf8(
			obs_data_get_string(data,
					    "selectedSecondaryTextSource"));
		displayDuration = (int)obs_data_get_int(
			data, "displayDuration"); // millisec

		// Write Data to inputfields
		ui->displayTimeSpinBox->setValue(displayDuration);

		int sceneIDX = ui->sceneDropdownList->findText(selectedScene);
		if (sceneIDX != -1) {
			ui->sceneDropdownList->setCurrentIndex(sceneIDX);
		}

		int folderIDX = ui->groupSourceDropdownList->findText(
			selectedGroupFolderSource);
		if (folderIDX != -1) {
			ui->groupSourceDropdownList->setCurrentIndex(folderIDX);
		}

		int mainTxtIDX = ui->mainTextSourceDropdownList->findText(
			selectedMainTextSource);
		if (mainTxtIDX != -1) {
			ui->mainTextSourceDropdownList->setCurrentIndex(
				mainTxtIDX);
		}

		int secTxtIDX = ui->secondaryTextSourceDropdownList->findText(
			selectedSecondaryTextSource);
		if (secTxtIDX != -1) {
			ui->secondaryTextSourceDropdownList->setCurrentIndex(
				secTxtIDX);
		}

		// write Items into lowerthirditems vector array
		lowerthirditems.clear();
		obs_data_array_t *dataArrayItems =
			obs_data_get_array(data, "lowerthirditems");
		for (int i = 0; i < (int)(obs_data_array_count(dataArrayItems));
		     i++) {
			obs_data_t *itemData =
				obs_data_array_item(dataArrayItems, i);
			LowerthirdswitcherDockWidget::lowerthirditem item;
			item.mainText = QString::fromUtf8(
				obs_data_get_string(itemData, "mainText"));
			item.secondaryText = QString::fromUtf8(
				obs_data_get_string(itemData, "secondaryText"));
			lowerthirditems.push_back(item);
			obs_data_release(itemData);
		}
		// load lowerthirditems to ui
		context->LoadItemsToList();
		if (ui->itemsListWidget->count() > 0) {
			ui->itemsListWidget->setCurrentRow(0);
			context->editItemClicked(ui->itemsListWidget->item(0));
			context->setActiveItem(0);
		}

		obs_data_array_release(dataArrayItems);
		obs_data_release(data);
	} else {
		displayDuration = ui->displayTimeSpinBox->value();
	}
}

void LowerthirdswitcherDockWidget::SaveSettings()
{
	obs_data_t *obsSettingsData = obs_data_create();
	obs_data_t *sceneCollectionData = obs_data_create();

	// check if settings file exists, if not create empty. Read it's data
	if (!(access(obs_module_config_path(CONFIG), F_OK) != -1))
		obs_data_save_json(
			obs_data_create(),
			obs_module_config_path(
				CONFIG)); // in case the file does not exist, create a empty json file
	obsSettingsData =
		obs_data_create_from_json_file(obs_module_config_path(CONFIG));

	obs_data_set_int(sceneCollectionData, "displayDuration",
			 displayDuration);
	obs_data_set_string(sceneCollectionData, "selectedScene",
			    selectedScene.toLocal8Bit().constData());
	obs_data_set_string(
		sceneCollectionData, "selectedGroupFolderSource",
		selectedGroupFolderSource.toLocal8Bit().constData());
	obs_data_set_string(sceneCollectionData, "selectedMainTextSource",
			    selectedMainTextSource.toLocal8Bit().constData());
	obs_data_set_string(
		sceneCollectionData, "selectedSecondaryTextSource",
		selectedSecondaryTextSource.toLocal8Bit().constData());

	obs_data_array_t *dataArrayItems = obs_data_array_create();

	for (std::vector<LowerthirdswitcherDockWidget::lowerthirditem>::iterator
		     it = lowerthirditems.begin();
	     it != lowerthirditems.end(); ++it) {
		obs_data_t *itemData = obs_data_create();
		obs_data_set_string(itemData, "mainText",
				    it->mainText.toLocal8Bit().constData());
		obs_data_set_string(
			itemData, "secondaryText",
			it->secondaryText.toLocal8Bit().constData());
		obs_data_array_push_back(dataArrayItems, itemData);
		obs_data_release(itemData);
	}

	obs_data_set_array(sceneCollectionData, "lowerthirditems",
			   dataArrayItems);
	obs_data_array_release(dataArrayItems);

	// Write settings specific to scene collection to settings
	obs_data_set_obj(obsSettingsData, currentSceneCollection,
			 sceneCollectionData);

	// Write Hotkey settings to global settings (applies to all scene collections)
	obs_data_array_t *start_countdown_hotkey_save_array =
		obs_hotkey_save(nextButtonHotkeyId);
	obs_data_set_array(obsSettingsData, "Next_Lower_Third_Hotkey",
			   start_countdown_hotkey_save_array);
	obs_data_array_release(start_countdown_hotkey_save_array);

	char *file = obs_module_config_path(CONFIG);
	if (!obs_data_save_json(obsSettingsData, file)) {
		char *path = obs_module_config_path("");
		if (path) {
			os_mkdirs(path);
			bfree(path);
		}
		obs_data_save_json(obsSettingsData, file);
	}
	bfree(file);

	obs_data_release(sceneCollectionData);
	obs_data_release(obsSettingsData);
}

void LowerthirdswitcherDockWidget::setActiveItem(int i)
{
	activeItem = i;
}

void LowerthirdswitcherDockWidget::setCurrentSceneCollection()
{
	currentSceneCollection = obs_frontend_get_current_scene_collection();
}

const char *LowerthirdswitcherDockWidget::getCurrentSceneCollection()
{
	return currentSceneCollection;
}
