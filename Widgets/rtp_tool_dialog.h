#ifndef RTP_TOOL_DIALOG_H
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

class MyThread;

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

    void onUpdateProgress(int value);

private:
    QVector<QString> getAttributes(Element ele, const QVector<QString>& attrs);

    bool constructElementsFromXlsx(
        const QString& path,
        vector<QString>& header,
        vector<Element>& content
    );

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

    // Configurations and output
    FilePathEditor* output_file_;

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
    ReactionSearchDecluster* processsor_;

    // Table viewer for result display
    TableViewer* result_viewer_;

    // Thread
    MyThread* my_thread_;
};

class MyThread : public QThread
{
    Q_OBJECT
public:
    MyThread(RTPToolDialog* creator, QObject* parent = nullptr);

    void startWork();
    void run();

signals:
    void progress(int value);

private:
    RTPToolDialog* rtp_dialog_;
};

#endif // RTP_TOOL_DIALOG_H
