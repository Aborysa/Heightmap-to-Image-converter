#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QButtonGroup>
#include <QFileSystemWatcher>
#define MODE_COUNT 2
#define TO_IMG 0
#define TO_HGT 1

namespace Ui {
class MainWindow;
class HeightDiag;
}
class HeightDiag : public QDialog{
    Q_OBJECT

};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_hgtBttn_clicked();
    void on_imgBttn_clicked();
    void on_hgtDiag_currentChanged(const QString & path);
    void on_imgDiag_currentChanged(const QString & path);
    void on_modeChange(QAbstractButton * bttn);
    void on_modeBttn_clicked();

    void on_convertBttn_clicked();

    void on_toolButton_clicked();

    void on_toolButton_2_clicked();

    void on_toolButton_3_clicked();

    void on_toolButton_5_clicked();

    void on_smoothSlider_valueChanged(int value);

    void on_modSlider_valueChanged(int value);

    void on_fileChange(const QString &path);
private:
    Ui::MainWindow * ui;
    QFileDialog * hgtDiag;
    QFileDialog * imgDiag;
    QString hgtFile;
    QString imgFile;
    bool fileHasChanged = false;
    bool converted = false;
    QDir sharedPath = QDir::currentPath();
    QButtonGroup * channelG;
    QButtonGroup * modeG;
    QFileSystemWatcher fileWatcher;
    int mode = TO_IMG;
    bool processing = false;

};

#endif // MAINWINDOW_H
