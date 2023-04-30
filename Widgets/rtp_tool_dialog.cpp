#include "rtp_tool_dialog.h"

// Test
#include <QDebug>
#include <mutex>
#include <thread>

using namespace std;

RTPToolDialog::RTPToolDialog(QWidget *parent) :
    QMainWindow(parent),
    processsor_(nullptr),
    result_viewer_(nullptr)
{
    QVBoxLayout* dialog_layout = new QVBoxLayout();
    QWidget* window = new QWidget();
    window->setLayout(dialog_layout);
    setCentralWidget(window);

    // Set up menu bar
    menu_bar_ = new QMenuBar(this);
    this->setMenuBar(menu_bar_);
    menuBar()->setNativeMenuBar(false); // Need this line, otherwise the menu bar not showing

    QMenu* menu_file = new QMenu("File");
    menu_bar_->addMenu(menu_file);
    QAction* action_export_settings = new QAction("Export settings...");
    QAction* action_import_settings = new QAction("Import settings...");
    menu_file->addAction(action_export_settings);
    menu_file->addAction(action_import_settings);

    connect(action_export_settings, SIGNAL(triggered(bool)), this, SLOT(onExportSettings()));
    connect(action_import_settings, SIGNAL(triggered(bool)), this, SLOT(onLoadSettings()));

    dialog_layout->setSpacing(0);
    dialog_layout->setMargin(0);

    QWidget* setting_area = new QWidget();
    QVBoxLayout* setting_area_layout = new QVBoxLayout();
    setting_area->setLayout(setting_area_layout);
    dialog_layout->addWidget(setting_area);

    QScrollArea* scroll_area = new QScrollArea(setting_area);
    setting_area_layout->addWidget(scroll_area);

    QWidget* scroll_area_widget_content = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(scroll_area_widget_content);

    // File selection
    QGroupBox* grp_files = new QGroupBox("Input files", this);
    layout->addWidget(grp_files);

    QVBoxLayout* grp_files_layout = new QVBoxLayout();
    grp_files->setLayout(grp_files_layout);
    grp_files_layout->setMargin(0);

    input_file_ = new FilePathEditor("Input");
    grp_files_layout->addWidget(input_file_);

    database_file_ = new FilePathEditor("Database");
    grp_files_layout->addWidget(database_file_);

    pos_file_ = new FilePathEditor("Positive list");
    grp_files_layout->addWidget(pos_file_);

    neg_file_ = new FilePathEditor("Negative list");
    grp_files_layout->addWidget(neg_file_);

    adduct_list_file_ = new FilePathEditor("Adduct list");
    grp_files_layout->addWidget(adduct_list_file_);

    // Reaction search settings
    QGroupBox* grp_reaction_search_params = new QGroupBox("Reaction search settings", this);
    layout->addWidget(grp_reaction_search_params);

    QHBoxLayout* grp_reaction_search_params_layout = new QHBoxLayout(grp_reaction_search_params);

    grp_reaction_search_params_layout->addWidget(new QLabel("Positive"));
    QButtonGroup* rs_true_false = new QButtonGroup(this);
    rs_true_btn_ = new QRadioButton("Yes");
    rs_true_btn_->setChecked(true);
    rs_true_false->addButton(rs_true_btn_);
    rs_false_btn_ = new QRadioButton("No");
    rs_true_false->addButton(rs_false_btn_);
    grp_reaction_search_params_layout->addWidget(rs_true_btn_);
    grp_reaction_search_params_layout->addWidget(rs_false_btn_);

    rs_threshold_ = new LabelValueEditor("Threshold");
    rs_threshold_->setValidator(new QDoubleValidator());
    grp_reaction_search_params_layout->addWidget(rs_threshold_);

    rs_m_prime_ = new LabelValueEditor("m'");
    rs_m_prime_->setValidator(new QDoubleValidator());
    grp_reaction_search_params_layout->addWidget(rs_m_prime_);

    rs_nb_combination_ = new LabelValueEditor("No. of combinations");
    rs_nb_combination_->setValidator(new QIntValidator());
    grp_reaction_search_params_layout->addWidget(rs_nb_combination_);


    // Pair peaking settings
    QGroupBox* grp_pair_peaking_params = new QGroupBox("Pair peaking settings");
    QHBoxLayout* grp_pair_peaking_params_layout = new QHBoxLayout(grp_pair_peaking_params);
    grp_pair_peaking_params_layout->setMargin(0);

    layout->addWidget(grp_pair_peaking_params);

    pp_mass_difference_ = new LabelValueEditor("Mass difference");
    pp_mass_difference_->setValidator(new QDoubleValidator());
    grp_pair_peaking_params_layout->addWidget(pp_mass_difference_);

    pp_mass_accuracy_ = new LabelValueEditor("Mass accuracy");
    pp_mass_accuracy_->setValidator(new QDoubleValidator());
    grp_pair_peaking_params_layout->addWidget(pp_mass_accuracy_);

    pp_rt_ = new LabelValueEditor("Retention time");
    pp_rt_->setValidator(new QDoubleValidator());
    grp_pair_peaking_params_layout->addWidget(pp_rt_);

    pp_mean_ = new LabelValueEditor("Mean");
    pp_mean_->setValidator(new QDoubleValidator());
    grp_pair_peaking_params_layout->addWidget(pp_mean_);

    // Adduct declustering parameter
    QGroupBox* grp_ad_settings = new QGroupBox("Adduct declustering settings");
    QHBoxLayout* grp_ad_settings_layout = new QHBoxLayout(grp_ad_settings);
    grp_ad_settings_layout->setMargin(0);

    layout->addWidget(grp_ad_settings);

    ad_mass_accuracy_ = new LabelValueEditor("Adduct mass accuracy");
    ad_mass_accuracy_->setValidator(new QDoubleValidator());
    grp_ad_settings_layout->addWidget(ad_mass_accuracy_);
    grp_ad_settings_layout->addStretch();

    // Computation settings
    QGroupBox* grp_compute_settings = new QGroupBox("Computation settings");
    QVBoxLayout* grp_compute_settings_layout = new QVBoxLayout(grp_compute_settings);
    layout->addWidget(grp_compute_settings);

    QHBoxLayout* num_threads_layout = new QHBoxLayout();
    num_threads_layout->addWidget(new QLabel("Number of threads"));
    num_threads_select_ = new QComboBox(this);
    QStringList num_thread_items;
    num_thread_items << "1" << "2" << "4" << "6" << "8" << "10";
    num_threads_select_->addItems(num_thread_items);
    num_threads_select_->setCurrentIndex(0);
    num_threads_layout->addWidget(num_threads_select_);
    grp_compute_settings_layout->addLayout(num_threads_layout);
    num_threads_layout->addStretch();

    // Export and load configuration buttons
    QHBoxLayout* btn_grp_layout = new QHBoxLayout();
    dialog_layout->addLayout(btn_grp_layout);

    // Run button
    run_btn_ = new QPushButton("Run");
    btn_grp_layout->addWidget(run_btn_);
    connect(run_btn_, SIGNAL(clicked()), this, SLOT(onRunBtnClicked()));

    // View results button
    view_result_btn_ = new QPushButton("View Results");
    btn_grp_layout->addWidget(view_result_btn_);
    connect(view_result_btn_, SIGNAL(clicked()), this, SLOT(onViewResultBtnClicked()));

    QStatusBar* status_bar_ = new QStatusBar();
    this->setStatusBar(status_bar_);

    progress_bar_ = new QProgressBar();
    status_bar_->addPermanentWidget(progress_bar_);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);
    progress_bar_->setTextVisible(true);

    status_label_ = new QLabel("This is status bar");
    status_bar_->addWidget(status_label_);

    scroll_area->setWidget(scroll_area_widget_content);

    my_thread_ = new MyThread(this);
    connect(my_thread_, &MyThread::progress, this, &RTPToolDialog::onUpdateProgress);
}

RTPToolDialog::~RTPToolDialog()
{
    delete my_thread_;
    if (processsor_ != nullptr)
        delete processsor_;
}

void RTPToolDialog::onExportSettings()
{
    QString path = QFileDialog::getSaveFileName(
        this, tr("Save File")
    );

    if (path.isEmpty())
        return;

    QFile file(path);
    if (file.exists()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this, "Warning", "A file with the same name already exists. Do you want to overwrite?",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::No)
            return;
    }

    file.open(QIODevice::ReadWrite | QIODevice::Text);
    QMap<QString, Parameter>::iterator i;
    for (i = parameter_map.begin(); i != parameter_map.end(); ++i) {
        QString str = i.key();
        Parameter val = i.value();
        QString line = str + "=";
        switch(val) {
            case INPUT_FILE:
                line += input_file_->getFilePath();
                break;
            case DATABASE_FILE:
                line += database_file_->getFilePath();
                break;
            case POSITIVE_LIST:
                line += pos_file_->getFilePath();
                break;
            case NEGATIVE_LIST:
                line += neg_file_->getFilePath();
                break;
            case ADDUCT_LIST:
                line += adduct_list_file_->getFilePath();
                break;
            case IS_POSITIVE:
                line += rs_true_btn_->isChecked()? "True" : "False";
                break;
            case RS_THRESHOLD:
                line += rs_threshold_->getText();
                break;
            case RS_M_PRIME:
                line += rs_m_prime_->getText();
                break;
            case RS_NUM_COMBINATIONS:
                line += rs_nb_combination_->getText();
                break;
            case PP_MASS_DIFF:
                line += pp_mass_difference_->getText();
                break;
            case PP_MASS_ACCURACY:
                line += pp_mass_accuracy_->getText();
                break;
            case PP_RT:
                line += pp_rt_->getText();
                break;
            case PP_MEAN:
                line += pp_mean_->getText();
                break;
            case AD_MASS_ACCURACY:
                line += ad_mass_accuracy_->getText();
                break;
            default:
                break;
        }
        line += "\n";
        file.write(line.toUtf8());
    }
    QMessageBox::information(this, tr("Configuration Export"),tr("Configuration exported successfully!"));
}

void RTPToolDialog::onLoadSettings()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("Load File")
    );

    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.exists()) {
        QMessageBox::information(
            this,
            tr("Configuration Import"),
            tr("Configuration file doesn't exist!")
        );
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(
            this,
            tr("Configuration Import"),
            tr("Can't open the file!")
        );
        return;
    }

    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        // format of str, e.g., INPUT_FILE=./xx/xxx/xx\n
        // Need to split at the '=' mark and remove '\n' at the end
        str.remove('\r');
        str.remove('\n');
        QStringList list = str.split("=");
        if (list.size() != 2)
            continue;
        QMap<QString, Parameter>::iterator iter;
        iter = parameter_map.find(list[0]);
        if (iter == parameter_map.end())
            continue;

        Parameter type = iter.value();
        QString value = list[1];
        switch(type) {
            case INPUT_FILE:
                input_file_->setFilePath(value);
                break;
            case DATABASE_FILE:
                database_file_->setFilePath(value);
                break;
            case POSITIVE_LIST:
                pos_file_->setFilePath(value);
                break;
            case NEGATIVE_LIST:
                neg_file_->setFilePath(value);
                break;
            case ADDUCT_LIST:
                adduct_list_file_->setFilePath(value);
                break;
            case IS_POSITIVE:
                if (value == "True")
                    rs_true_btn_->setChecked(true);
                else
                    rs_false_btn_->setChecked(true);
                break;
            case RS_THRESHOLD:
                rs_threshold_->setText(value);
                break;
            case RS_M_PRIME:
                rs_m_prime_->setText(value);
                break;
            case RS_NUM_COMBINATIONS:
                rs_nb_combination_->setText(value);
                break;
            case PP_MASS_DIFF:
                pp_mass_difference_->setText(value);
                break;
            case PP_MASS_ACCURACY:
                pp_mass_accuracy_->setText(value);
                break;
            case PP_RT:
                pp_rt_->setText(value);
                break;
            case PP_MEAN:
                pp_mean_->setText(value);
                break;
            case AD_MASS_ACCURACY:
                ad_mass_accuracy_->setText(value);
                break;
            default:
                break;
        }
    }

    QMessageBox::information(
        this,
        tr("Configuration Import"),
        tr("Configurations imported successfully!")
    );
    return;
}

void RTPToolDialog::onUpdateProgress(int value)
{
    progress_bar_->setValue(value);
}

void RTPToolDialog::onRunBtnClicked()
{
    /*
    //qDebug() << "Is runing?" << my_thread_->isRunning();
    my_thread_->start();
    //qDebug() << "Is runing?" << my_thread_->isRunning();
    return;
    */

    // Read input file
    QString input_file_path = input_file_->getFilePath();
    vector<QString> input_header;
    vector<Element> input_content;
    if (!constructElementsFromXlsx(input_file_path, input_header, input_content)) {
        QMessageBox::information(
            this,
            tr("RTP tool"),
            tr("Invalid input file.") );
        return;
    }

    // Save the attribute names
    input_file_attrs_.clear();
    input_file_attrs_ = QVector<QString>::fromStdVector(input_header);

    // Read the database file
    QString database_file_path = database_file_->getFilePath();
    vector<QString> database_header;
    vector<Element> database_content;
    if (!constructElementsFromXlsx(database_file_path, database_header, database_content)) {
        QMessageBox::information(
            this,
            tr("RTP tool"),
            tr("Invalid database file.") );
        return;
    }

    // Read positive, negative, and adduct lists
    QString pos_file_path = pos_file_->getFilePath();
    vector<QString> pos_header;
    vector<Element> pos_content;
    if (!constructElementsFromXlsx(pos_file_path, pos_header, pos_content)) {
        QMessageBox::information(
            this,
            tr("RTP tool"),
            tr("Invalid positive list file.") );
        return;
    }

    QString neg_file_path = neg_file_->getFilePath();
    vector<QString> neg_header;
    vector<Element> neg_content;
    if (!constructElementsFromXlsx(neg_file_path, neg_header, neg_content)) {
        QMessageBox::information(
            this,
            tr("RTP tool"),
            tr("Invalid negative list file.") );
        return;
    }

    QString adduct_list_file_path = adduct_list_file_->getFilePath();
    vector<QString> adduct_list_header;
    vector<Element> adduct_list_content;
    if (!constructElementsFromXlsx(adduct_list_file_path, adduct_list_header, adduct_list_content)) {
        QMessageBox::information(
            this,
            tr("RTP tool"),
            tr("Invalid adduct list file.") );
        return;
    }

    // Read pair peaking parameters
    bool ok = false;
    double pp_mass_diff = pp_mass_difference_->getText().toDouble(&ok);
    if (!ok) {
        QMessageBox::information(this, tr("RTP tool"),tr("Invalid pair peaking mass difference.") );
        return;
    }

    double pp_mass_accuracy = pp_mass_accuracy_->getText().toDouble(&ok);
    if (!ok) {
        QMessageBox::information(this, tr("RTP tool"),tr("Invalid pair peaking mass accuracy.") );
        return;
    }

    double pp_rt = pp_rt_->getText().toDouble(&ok);
    if (!ok) {
        QMessageBox::information(this, tr("RTP tool"),tr("Invalid pair peaking retention time.") );
        return;
    }

    double pp_mean = pp_mean_->getText().toDouble(&ok);
    if (!ok) {
        QMessageBox::information(this, tr("RTP tool"),tr("Invalid pair peaking mean.") );
        return;
    }

    PairPeakingParameters pp_params = PairPeakingParameters(
        pp_mass_diff, pp_mass_accuracy, pp_rt, pp_mean
    );

    // Read adduct declustering parameter
    double adduct_mass_accuracy = ad_mass_accuracy_->getText().toDouble(&ok);
    if (!ok) {
        QMessageBox::information(this, tr("RTP tool"),tr("Invalid adduct declustering mass accuracy.") );
        return;
    }

    // Read reaction search parameters
    bool rs_is_positive = rs_true_btn_->isChecked()? true : false;

    double rs_threshold = rs_threshold_->getText().toDouble(&ok);
    if (!ok) {
        QMessageBox::information(this, tr("RTP tool"),tr("Invalid reaction search threshold.") );
        return;
    }

    double rs_m_prime = rs_m_prime_->getText().toDouble(&ok);
    if (!ok) {
        QMessageBox::information(this, tr("RTP tool"),tr("Invalid reaction search m'.") );
        return;
    }

    int rs_nb_combination = rs_nb_combination_->getText().toInt(&ok);
    if (!ok) {
        QMessageBox::information(this, tr("RTP tool"),tr("Invalid reaction search number of combinations.") );
        return;
    }

    ReactionSearchParameters rs_params = ReactionSearchParameters(
        rs_is_positive, rs_threshold, rs_m_prime, rs_nb_combination
    );

    if (processsor_ != nullptr) {
        delete processsor_;
        processsor_ = nullptr;
    }

    int num_threads = num_threads_select_->currentText().toInt();

    processsor_ = new ReactionSearchDecluster(
        input_content,
        database_content,
        pp_params,
        rs_params,
        adduct_mass_accuracy,
        pos_content,
        neg_content,
        adduct_list_content,
        num_threads
    );

    // JJ: need to change it to a status bar message.
    QMessageBox::information(this, tr("RTP tool"),tr("Process done!") );
}

/*
 * Slot function for view_result_btn_ "clicked" signal.
 * This function will pop up a new window with a table to show the reaction search and declustering results.
 */
void RTPToolDialog::onViewResultBtnClicked()
{
    if (processsor_ == nullptr)
        return;

    // Prepare table content for display.
    QVector<QString> attr_to_display = input_file_attrs_;
    QVector<QVector<QString>> contents;
    ReactionSearchDecluster::RSDResult result = processsor_->getRSDResult();
    vector<ReactionSearchDecluster::RetentionTimeGroup> rt_groups = result.rt_groups;
    for (int i = 0; i < rt_groups.size(); ++i) {
        vector<ReactionSearchDecluster::MassDiffGroup> mass_diff_groups = rt_groups[i].mass_diff_groups;
        vector<vector<string>> formulas = rt_groups[i].formulas;
        for (int j = 0; j < mass_diff_groups.size(); ++j) {
            // Formula result of current mass diff group
            QString formula_result;
            for (int m = 0; m < formulas.size(); ++m) {
                formula_result += QString("%1 [%2];").arg(QString::number(m+1)).arg(QString::fromStdString(formulas[m][j]));
            }

            vector<vector<ReactionSearchResult>> rs_results = mass_diff_groups[j].rs_results;

            vector<ReactionSearchDecluster::IsotopePairObject> paired_objects = mass_diff_groups[j].paired_objects;
            for (int k = 0; k < paired_objects.size(); ++k) {
                ReactionSearchDecluster::IsotopePairObject paired_obj = paired_objects[k];
                QVector<QString> ele1_values = getAttributes(paired_obj.attributes.ele1, attr_to_display);
                QVector<QString> ele2_values = getAttributes(paired_obj.attributes.ele2, attr_to_display);

                QString rs_result;
                // Reaction search results
                for (int m = 0; m < rs_results[k].size(); ++m) {
                    rs_result += QString("%1 [%2, %3, %4]; ").arg(QString::number(m+1)).arg(QString::fromStdString(rs_results[k][m].value)).arg(QString::fromStdString(rs_results[k][m].type)).arg(QString::number(rs_results[k][m].accuracy));
                }

                ele1_values.push_back(formula_result);
                ele1_values.push_back(rs_result);

                ele2_values.push_back(formula_result);
                ele2_values.push_back(rs_result);

                contents.push_back(ele1_values);
                contents.push_back(ele2_values);
            }
        }
    }

    // Prepare reaction search results
    attr_to_display.push_back("Formulas");
    attr_to_display.push_back("Reaction search results");
    if (result_viewer_ != nullptr) {
        result_viewer_->close();
        result_viewer_->updateTable(attr_to_display, contents);
    } else {
        result_viewer_ = new TableViewerDialog(this);
        result_viewer_->updateTable(attr_to_display, contents);
    }
    result_viewer_->show();
}

/*
 * Get the values of the input attributes (attrs) of the input Element (ele).
 */
QVector<QString> RTPToolDialog::getAttributes(Element ele, const QVector<QString>& attrs)
{
    QVector<QString> values(attrs.size(), QString());
    for (int i = 0; i < attrs.size(); ++i)
        values[i] = QString::fromStdString(ele.getPropertyValue(attrs[i].toStdString()));
    return values;
}

bool RTPToolDialog::constructElementsFromXlsx(
    const QString& path,
    vector<QString>& header,
    vector<Element>& content
) {
    XlsxIO reader = XlsxIO(path);
    // The Xlsx file must have a header line
    vector<vector<QString>> raw_content;
    bool success = reader.read(header, raw_content, true); // first row is header line.

    if (!success)
        return false;

    int nb_rows = raw_content.size();
    int nb_columns = header.size();
    content.reserve(nb_rows);
    for (int i = 0; i < nb_rows; ++i) {
        Element ele;
        for (int j = 0; j < nb_columns; ++j) {
            //QString attr_name = header[j];
            //QString value = raw_content[i][j];
            ele.addProperty(header[j].toStdString(), raw_content[i][j].toStdString());
        }
        content.push_back(ele);
    }
    return true;
}


/*
    Class: MyThread
*/
MyThread::MyThread(RTPToolDialog *creator, QObject *parent) : QThread(parent)
{
    rtp_dialog_ = creator;
}

void MyThread::startWork()
{
    for (int i = 1; i <= 10; ++i) {
        QThread::sleep(2);
        emit progress(10 * i);
    }

    emit progress(100);
}

void MyThread::run()
{
    startWork();
}
