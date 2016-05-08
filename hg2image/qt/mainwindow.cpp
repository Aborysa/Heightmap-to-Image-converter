#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include "heightMap.h"
#include <QImage>
#include <QImageWriter>
#include <QImageReader>
#include <QRgb>
#include <math.h>
bool isSupportedFormat(QString format)
{
    bool supp = false;
    for(int i=0; i<QImageWriter::supportedImageFormats().length();i++)
    {
        supp |= !QString::compare(format,QImageWriter::supportedImageFormats().at(i),Qt::CaseInsensitive);
    }
    return supp;

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    hgtDiag = new QFileDialog(this);
    hgtDiag->setFileMode(QFileDialog::AnyFile);
    hgtDiag->setNameFilter(tr("Height maps (*.hg2 *.hgt)"));
    hgtDiag->setAcceptMode(QFileDialog::AcceptOpen);
    hgtDiag->setLabelText(QFileDialog::Accept,"Select");
    hgtDiag->setViewMode(QFileDialog::Detail);
    connect(hgtDiag,SIGNAL(fileSelected(const QString &)),this,SLOT(on_hgtDiag_currentChanged(const QString &)));

    imgDiag = new QFileDialog(this);
    imgDiag->setFileMode(QFileDialog::AnyFile);
    imgDiag->setNameFilter(tr("Images (*.png *.bmp)"));
    imgDiag->setAcceptMode(QFileDialog::AcceptOpen);
    imgDiag->setLabelText(QFileDialog::Accept,"Select");
    imgDiag->setViewMode(QFileDialog::Detail);
    connect(imgDiag,SIGNAL(fileSelected(const QString &)),this,SLOT(on_imgDiag_currentChanged(const QString &)));

    channelG = new QButtonGroup(this);
    modeG = new QButtonGroup(this);


    channelG->addButton(ui->channelRed,2);
    channelG->addButton(ui->channelGreen,1);
    channelG->addButton(ui->channelBlue,0);

    modeG->addButton(ui->modePrecisionColor,0);
    modeG->addButton(ui->modeNoloss,1);
    modeG->addButton(ui->modeOneChannel,2);
    modeG->addButton(ui->modeOneChannelS,3);
    modeG->addButton(ui->modeCustom,4);
    ui->channelGroup->setEnabled(false);
    connect(&fileWatcher,SIGNAL(fileChanged(const QString &)),this,SLOT(on_fileChange(const QString &)));
    connect(modeG,SIGNAL(buttonClicked(QAbstractButton *)),this,SLOT(on_modeChange(QAbstractButton *)));

}
void MainWindow::on_modeChange(QAbstractButton * bttn){
    bool e = bttn == ui->modeOneChannel || bttn == ui->modeOneChannelS;
    ui->channelGroup->setEnabled(e);
    ui->smoothSlider->setEnabled(bttn == ui->modeOneChannelS);
}
MainWindow::~MainWindow()
{
    delete modeG;
    delete channelG;
    delete hgtDiag;
    delete imgDiag;
    delete ui;
}

void MainWindow::on_hgtBttn_clicked()
{
    hgtDiag->setDirectory(sharedPath);
    hgtDiag->show();
}
void MainWindow::on_hgtDiag_currentChanged(const QString & path)
{
    ui->hgtBttn->setText(path.split("/").last());
    sharedPath = hgtDiag->directory();
    if(!hgtFile.isEmpty()){
        fileWatcher.removePath(path);
    }
    if(fileHasChanged){
        setWindowTitle(windowTitle().remove(0,1));
    }
    fileHasChanged = false;
    hgtFile = path;

    fileWatcher.addPath(path);
}
void MainWindow::on_imgDiag_currentChanged(const QString & path)
{
    std::cout << "format: " << path.split(".").last().toStdString() << "\n";
    if(isSupportedFormat(path.split(".").last())){
        ui->imgBttn->setText(path.split("/").last());
        sharedPath = imgDiag->directory();
        if(!imgFile.isEmpty()){
            fileWatcher.removePath(path);
        }
        imgFile = path;
        if(fileHasChanged){
            setWindowTitle(windowTitle().remove(0,1));
        }
        fileHasChanged = false;
        fileWatcher.addPath(path);
    }else{
        QMessageBox::critical(this,"Unsupported","Image format is not supported!");
    }
}
void MainWindow::on_imgBttn_clicked()
{
    imgDiag->setDirectory(sharedPath);
    imgDiag->show();
}

void MainWindow::on_modeBttn_clicked()
{
    mode++;
    mode %= MODE_COUNT;
    QString tx;
    switch(mode){
    case 0:
        this->ui->modeBttn->setText("→");
        //ui->modeOneChannelS->setEnabled(false);
        tx.setNum(ui->modSlider->value());
        break;
    case 1:
        this->ui->modeBttn->setText("←");
        tx.setNum(1.0f/(float)ui->modSlider->value(),'g',2);
        //ui->modeOneChannelS->setEnabled(true);
        break;
    }
    ui->modValue->setText(tx);

}

void MainWindow::on_convertBttn_clicked()
{
    /*Check if file is selected*/
    if(!processing){
        if(!(hgtFile.isEmpty() || imgFile.isEmpty()) ){
            processing = true;
            heightMap * hMap = nullptr;
            QImage * img = nullptr;
            bool failed = false;
            ui->progressBar->setValue(0);
            ui->status->setStyleSheet("color: orange;");
            int cMode = channelG->checkedId();
            int slider = ui->smoothSlider->value();
            QAbstractButton * convMode = modeG->checkedButton();
            if(mode == TO_IMG){
                //TODO: add a function for mapping to support differnet mapping functions
                ui->status->setText("Loading...");
                qApp->processEvents();
                hMap = new heightMap(hgtFile.toStdString().c_str());
                if(hMap->isValid()){
                    img = new QImage(hMap->getWidth(),hMap->getLength(),QImage::Format_RGBA8888);
                    long total = hMap->getLength()*hMap->getWidth();
                    for(int y=0; y < hMap->getLength();y++){
                        ui->status->setText("Converting" + QString(".").repeated(((int)(y/128))%4));
                        for(int x=0; x < hMap->getWidth();x++){
                            float p = (hMap->getWidth() * 100.0f)/(float)(total) * y;
                            ui->progressBar->setValue(p);

                            unsigned int mapped = hMap->getHeight(x,y);
                            mapped *= ui->modSlider->value();
                            if(convMode == ui->modeOneChannel){
                                mapped = ((mapped*255)/4095) << (8*cMode);
                            }
                            else if(convMode == ui->modeNoloss){
                                mapped <<= 8;
                            }
                            else if(convMode == ui->modePrecisionColor){
                                //GGGGGGG RRRR
                                mapped = ((mapped & 0xF) << 16) | ((mapped & 0xFF0) << 4);
                            }else if(convMode == ui->modeOneChannelS){
                                mapped = ((mapped  >> 4) & 0xFF) << (8*cMode);
                            }
                            //unsigned short mapped = (hMap->getHeight(x,y)*255)/4095;

                            img->setPixel(x,hMap->getLength()-(y+1),(0xFF000000 | mapped));
                        }
                        qApp->processEvents();
                    }
                    QImageWriter writer(imgFile);
                    writer.setFormat(((QString)imgFile.split(".").last()).toUtf8());
                    ui->status->setText("Saving...");
                    qApp->processEvents();
                    writer.write(*img);
                }else{
                    failed = true;
                    QMessageBox::critical(this,"Invalid!","Invalid hg2 file!");
                }
            }else if(mode == TO_HGT){
                ui->status->setText("Loading...");
                qApp->processEvents();
                QImageReader reader(imgFile);
                img = new QImage(reader.size(),QImage::Format_RGBA8888);
                reader.read(img);
                hMap = new heightMap(img->width()/256,img->height()/256,8);
                heightMap * oMap = new heightMap(hgtFile.toStdString().c_str());
                if(oMap->isValid()){
                    hMap->setRev(oMap->getRev());
                }
                delete oMap;
                long total = hMap->getLength()*hMap->getWidth();
                std::cout << "Converting started!\n";
                for(int y=0; y < hMap->getLength();y++){
                    ui->status->setText("Converting" + QString(".").repeated(((int)(y/128))%4));
                    for(int x=0; x < hMap->getWidth();x++){
                        float p = (hMap->getWidth() * 100.0f)/(float)(total) * y;
                        ui->progressBar->setValue(p);
                        QRgb px = img->pixel(x,hMap->getLength()-(y+1));
                        unsigned int mapped = px;
                        if(convMode == ui->modeOneChannel){
                            mapped = (((mapped >> (8*cMode))&0xFF)*4095)/255;
                        }
                        if(convMode == ui->modeNoloss){
                            mapped >>= 8;
                        }
                        if(convMode == ui->modePrecisionColor){
                            //GGGGGGG RRRR
                            mapped = ((mapped >> 16) & 0xF) | ((mapped >> 4) & 0xFF0);
                        }
                        if(convMode == ui->modeOneChannelS){
                            mapped = (((mapped >> (cMode*8)) & 0xFF) << 4 );
                        }
                        mapped *= (1.0f/ui->modSlider->value());
                        if(mapped >= 4096){
                            mapped = 4095;
                        }
                        hMap->setHeight(x,y,mapped);
                    }
                    qApp->processEvents();
                }
                if(convMode == ui->modeOneChannelS){
                    std::cout << "Smoothing!\n";
                    heightMap * sHGT = new heightMap(img->width()/256,img->height()/256,8);
                    sHGT->setRev(hMap->getRev());
                    for(int y=0; y < hMap->getLength();y++){
                        ui->status->setText("Smoothing" + QString(".").repeated(((int)(y/128))%4));
                        for(int x=0; x < hMap->getWidth();x++){
                            float p = (hMap->getWidth() * 100.0f)/(float)(total) * y;
                            float tVal = 0;
                            for(int ry = -slider;ry < slider+1;ry++){
                                for(int rx = -slider; rx < slider+1; rx++){
                                    tVal += hMap->getHeight(rx+x,ry+y);
                                }
                            }
                            tVal /= pow(1+slider*2,2);
                            ui->progressBar->setValue(p);
                            sHGT->setHeight(x,y,round(tVal));
                        }
                        qApp->processEvents();
                    }
                    delete hMap;
                    hMap = sHGT;
                }
                ui->status->setText("Saving...");
                qApp->processEvents();
                hMap->save(hgtFile.toStdString().c_str(),false);
            }
            if(img != nullptr){delete img;}
            if(hMap != nullptr){delete hMap;}
            if(!failed){
                ui->status->setStyleSheet("color:green;");
                ui->status->setText("Done!");
                ui->progressBar->setValue(100);
            }else{
                ui->status->setStyleSheet("color:red;");
                ui->status->setText("Error!");
                ui->progressBar->setValue(0);
            }
            processing = false;
            converted = true;
        }else{
            ui->status->setStyleSheet("color:red;");
            ui->status->setText("Error!");
            QMessageBox::critical(this,"Choose path!","hg2 or img file not chosen!");
        }
    }
    if(fileHasChanged){
        setWindowTitle(windowTitle().remove(0,1));
    }
    fileHasChanged = false;

    //setWindowTitle(windowTitle());

}
void MainWindow::on_fileChange(const QString &path)
{
    if(!converted){
        if(!fileHasChanged){
            setWindowTitle("*" + windowTitle());
        }
        fileHasChanged = true;
        if(ui->auto_convert_check->isChecked()){
              if((mode == TO_IMG && hgtFile==path)  || (mode == TO_HGT && imgFile==path) ){
                on_convertBttn_clicked();
            }
        }
    }
    converted = false;
}
void MainWindow::on_toolButton_clicked()
{
    QMessageBox::information(this,"Precision color mode","This mode will map the red channel to the 4 lower bits while\n"
                                                         "the green channel is maped to the 8 upper bits, each increment\n"
                                                         "of red = 0.1 meters while each increment of green = 1.6 meters");
}

void MainWindow::on_toolButton_2_clicked()
{
    QMessageBox::information(this,"Direct mapping","This mode will map the green channel to the 8 lower bits while\n"
                                                         "the red channel is maped to the 4 upper bits, each increment\n"
                                                         "of red = 25.6 meters while each increment of green = 0.1 meters");
}

void MainWindow::on_toolButton_3_clicked()
{
    QMessageBox::information(this,"One channel mappinge","This mode will map all the height data to one color channel\n"
                                                         "the mapping is done by multiplying the height data by 255 \n"
                                                         "and dividing by 4095, keep in mind that this mode is lossy.");

}

void MainWindow::on_toolButton_5_clicked()
{
    QMessageBox::information(this,"Smooth precision","This mode maps the color channel to the 8 upper bits, after converting\n"
                                                     "it will run a smoothing algorithm that smooths everything out.\n"
                                                     "When converting from hg2 to an image it will strip the lower 4 bits.");

}



void MainWindow::on_smoothSlider_valueChanged(int value)
{
    ui->smoothCount->setText(QString::number(value));
}


void MainWindow::on_modSlider_valueChanged(int value)
{
    QString tx;
    tx.setNum(value);
    if(mode == 1){
        tx.setNum(1.0f/(float)value,'g',2);
    }
    ui->modValue->setText(tx);
}
