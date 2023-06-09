﻿#ifndef RTP_TOOL_DIALOG_H
#define RTP_TOOL_DIALOG_H

// Widgets
#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QMessageBox>
#include <QMap>
#include <QScrollArea>
#include <QStatusBar>
#include <QProgressBar>
#include <QToolBar>
#include <QMenuBar>
#include <QMainWindow>
#include <QComboBox>
#include <QThread>
#include "file_path_editor.h"
#include "Widgets/label_value_editor.h"
#include "table_viewer.h"

// File input and ouput
#include "FileIO/file_io.h"

// Others
#include <vector>
#include "DataModel/element.h"
#include "Processing/reaction_search_decluster.h"

class RTPRunner;

class RTPToolDialog : public QMainWindow
{
    Q_OBJECT
public:
    enum Parameter {
        INPUT_FILE,
        DATABASE_FILE,
        POSITIVE_LIST,
        NEGATIVE_LIST,
        ADDUCT_LIST,
        IS_POSITIVE,
        RS_THRESHOLD,
        RS_M_PRIME,
        RS_NUM_COMBINATIONS,
        PP_MASS_DIFF,
        PP_MASS_ACCURACY,
        PP_RT,
        PP_MEAN,
        // Adduct declustering parameter
        AD_MASS_ACCURACY
    };

    QMap<QString, Parameter> parameter_map = {
        // Input files
        {"INPUT_FILE", INPUT_FILE},
        {"DATABASE_FILE", DATABASE_FILE},
        {"POSITIVE_LIST", POSITIVE_LIST},
        {"NEGATIVE_LIST", NEGATIVE_LIST},
        {"ADDUCT_LIST", ADDUCT_LIST},
        // Reaction search params
        {"IS_POSITIVE", IS_POSITIVE},
        {"RS_THRESHOLD", RS_THRESHOLD},
        {"RS_M_PRIME", RS_M_PRIME},
        {"RS_NUM_COMBINATIONS", RS_NUM_COMBINATIONS},
        // Pair peaking parameters
        {"PP_MASS_DIFF", PP_MASS_DIFF},
        {"PP_MASS_ACCURACY", PP_MASS_ACCURACY},
        {"PP_RT", PP_RT},
        {"PP_MEAN", PP_MEAN},
        // Adduct declustering parameter
        {"AD_MASS_ACCURACY", AD_MASS_ACCURACY}
    };

    explicit RTPToolDialog(QWidget *parent = nullptr);
    ~RTPToolDialog();

signals:

public slots:
    void onRunBtnClicked();
    void onViewResultBtnClicked();
    void onExportSettings();
    void onLoadSettings();

    void onUpdateProgress(QString msg, int value);

private slots:
    void onProcessStart();
    void onProcessFinished();

private:
    QVector<QString> getAttributes(Element ele, const QVector<QString>& attrs);

    bool constructElementsFromXlsx(
        const QString& path,
        vector<QString>& header,
        vector<Element>& content
    );

    void setStatusMessage(QString msg);

    // Menu bar
    QMenuBar* menu_bar_;

    // Input files
    FilePathEditor* input_file_;
    FilePathEditor* database_file_;
    FilePathEditor* pos_file_;
    FilePathEditor* neg_file_;
    FilePathEditor* adduct_list_file_;

    // Reaction search parameters
    QRadioButton* rs_true_btn_;
    QRadioButton* rs_false_btn_;
    LabelValueEditor* rs_threshold_;
    LabelValueEditor* rs_m_prime_;
    LabelValueEditor* rs_nb_combination_;

    // Pair peaking parameters
    LabelValueEditor* pp_mass_difference_;
    LabelValueEditor* pp_mass_accuracy_;
    LabelValueEditor* pp_rt_;
    LabelValueEditor* pp_mean_;

    // Adduct declustering parameter
    LabelValueEditor* ad_mass_accuracy_;

    // Computation settings
    QComboBox* num_threads_select_;

    // Buttons
    QPushButton* run_btn_; // Click to run the algorithm
    QPushButton* view_result_btn_;

    // Status bar
    QStatusBar* status_bar_;
    QLabel* status_label_;
    QProgressBar* progress_bar_;

    // The attribute names of the input file (the header line)
    QVector<QString> input_file_attrs_;

    // RTP processor
    ReactionSearchDecluster* processor_;

    // Table viewer for result display
    TableViewerDialog* result_viewer_;

    // Thread
    RTPRunner* rtp_runner_;
};

class RTPRunner : public QThread
{
    Q_OBJECT
public:
    RTPRunner(RTPToolDialog* rtp_dialog, QObject* parent = nullptr);
    ~RTPRunner();

    void setData(
        const vector<Element>& input,
        const vector<Element>& database,
        const vector<Element>& positive,
        const vector<Element>& negative,
        const vector<Element>& adduct_list,
        const float& adduct_mass_accuracy,
        const PairPeakingParameters& pp_params,
        const ReactionSearchParameters& rs_params,
        const int& nb_threads
    );

    ReactionSearchDecluster* takeoverProcecssor();

    void run();

signals:
    void progress(int value);
    void processStart();
    void processFinished();

private:
    void startProcess();

    // RTP dialog where an instance of this runner is created
    RTPToolDialog* rtp_dialog_;

    // Save the RTP processor after finishing the process.
    ReactionSearchDecluster* processor_;

    // Data used for the process
    vector<Element> input_;
    vector<Element> database_;
    vector<Element> positive_;
    vector<Element> negative_;
    vector<Element> adduct_list_;
    float adduct_mass_accuracy_;
    PairPeakingParameters pp_params_;
    ReactionSearchParameters rs_params_;
    int nb_threads_;
};

#endif // RTP_TOOL_DIALOG_H
