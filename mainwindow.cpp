#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QFileDialog>

static const QSize resultSize(640, 480);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(OpenImage()));
    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(SaveImage()));
    connect(ui->saveDisplayButton, SIGNAL(clicked()), this, SLOT(SaveDisplayImage()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(ResetImage()));
    connect(ui->toggleButton, SIGNAL(pressed()), this, SLOT(ToggleImage()));
    connect(ui->toggleButton, SIGNAL(released()), this, SLOT(ToggleImage()));

    connect(ui->bwButton, SIGNAL(clicked()), this, SLOT(BlackWhiteImage()));
    connect(ui->noiseButton, SIGNAL(clicked()), this, SLOT(AddNoise()));
    connect(ui->meanButton, SIGNAL(clicked()), this, SLOT(MeanBlurImage()));
    connect(ui->medianButton, SIGNAL(clicked()), this, SLOT(MedianImage()));
    connect(ui->gaussianBlurButton, SIGNAL(clicked()), this, SLOT(GaussianBlurImage()));
    connect(ui->firstDerivxButton, SIGNAL(clicked()), this, SLOT(FirstDerivImage_x()));
    connect(ui->firstDerivyButton, SIGNAL(clicked()), this, SLOT(FirstDerivImage_y()));
    connect(ui->secondDerivButton, SIGNAL(clicked()), this, SLOT(SecondDerivImage()));
    connect(ui->sharpenButton, SIGNAL(clicked()), this, SLOT(SharpenImage()));
    connect(ui->sobelButton, SIGNAL(clicked()), this, SLOT(SobelImage()));
    connect(ui->bilateralButton, SIGNAL(clicked()), this, SLOT(BilateralImage()));
    connect(ui->halfButton, SIGNAL(clicked()), this, SLOT(HalfImage()));
    connect(ui->rotateButton, SIGNAL(clicked()), this, SLOT(RotateImage()));
    connect(ui->peaksButton, SIGNAL(clicked()), this, SLOT(FindPeaksImage()));
    connect(ui->houghButton, SIGNAL(clicked()), this, SLOT(HoughImage()));
    connect(ui->smartButton, SIGNAL(clicked()), this, SLOT(SmartKMeans()));
    connect(ui->randomButton, SIGNAL(clicked()), this, SLOT(RandomSeedImage()));
    connect(ui->pixelButton, SIGNAL(clicked()), this, SLOT(PixelSeedImage()));
    connect(ui->histogramButton, SIGNAL(clicked()), this, SLOT(HistogramSeedImage()));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(OpenImage()));
    connect(ui->zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(Zoom(int)));
    connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(Brightness(int)));
    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(Scroll(int)));
    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(Scroll(int)));

    ui->meanBox->setValue(2);
    ui->medianBox->setValue(2);
    ui->blurSpinBox->setValue(2.0);
    ui->firstDerivSpinBox->setValue(2.0);
    ui->secondDerivSpinBox->setValue(2.0);
    ui->sharpenSigmaSpinBox->setValue(2.0);
    ui->sharpenMagSpinBox->setValue(1.0);
    ui->bilateralSigmaSSpinBox->setValue(2.0);
    ui->bilateralSigmaISpinBox->setValue(20.0);
    ui->noiseSpinBox->setValue(10.0);
    ui->orientationSpinBox->setValue(10.0);
    ui->peakThresholdSpinBox->setValue(10.0);
    ui->colorNoiseCheckBox->setChecked(true);
    ui->zoomSlider->setValue(0);
    ui->brightnessSlider->setValue(0);
    ui->clusterBox->setValue(4);

    displayImage = QImage(ui->ImgDisplay->width(), ui->ImgDisplay->height(), QImage::Format_RGB32);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::DrawDisplayImage()
{
    int zoom = ui->zoomSlider->value()/20 + 1;
    double brightness = (double) ui->brightnessSlider->value()/200.0 + 1.0;
    int r, c;
    int wd = displayImage.width();
    int hd = displayImage.height();
    int ws = outImage.width();
    int hs = outImage.height();

    QRgb pixel;
    pixel = qRgb(240, 240, 240);
    displayImage.fill(pixel);
    double rgb[3];

    for(r=0;r<hd;r++)
    {
        QRgb *scanLine = reinterpret_cast<QRgb *>(displayImage.scanLine(r));
        int rs = (int) ((double) (r - hd/2)/ (double) zoom + yScroll);

        if(rs >= 0 && rs < hs)
        {
            QRgb *scanLineS;

            if(ui->toggleButton->isDown())
                scanLineS = reinterpret_cast<QRgb *>(inImage.scanLine(rs));
            else
                scanLineS = reinterpret_cast<QRgb *>(outImage.scanLine(rs));

            for(c=0;c<wd;c++)
            {
                int cs = (int) ((double) (c - wd/2)/ (double) zoom + xScroll);

                if(cs >= 0 && cs < ws)
                {
                    pixel = scanLineS[cs];
                    rgb[0] = qRed(pixel);
                    rgb[1] = qGreen(pixel);
                    rgb[2] = qBlue(pixel);

                    rgb[0] *= brightness;
                    rgb[1] *= brightness;
                    rgb[2] *= brightness;

                    rgb[0] = min(255.0, rgb[0]);
                    rgb[1] = min(255.0, rgb[1]);
                    rgb[2] = min(255.0, rgb[2]);

                    scanLine[c] = (uint) qRgb((int) rgb[0], (int) rgb[1], (int) rgb[2]);
                }
            }
        }
    }

    ui->ImgDisplay->setPixmap(QPixmap::fromImage(displayImage));

}

void MainWindow::Zoom(int val)
{
    int zoom = val/20 + 1;
    double c0 = (double) (ui->ImgDisplay->width()/(2*zoom));
    double c1 = (double) (outImage.width()) - c0;


    if(c0 < c1)
    {
        xScroll = min(c1, max(c0, xScroll));
        double newScrollX = (xScroll - c0)/(c1 - c0);
        newScrollX *= (double) ui->horizontalScrollBar->maximum();

        zoomChanged = true;
        ui->horizontalScrollBar->setVisible(true);
        ui->horizontalScrollBar->setValue((int) newScrollX);
    }
    else
    {
        xScroll = (double) outImage.width()/2.0;
        ui->horizontalScrollBar->setVisible(false);
    }

    double r0 = (double) (ui->ImgDisplay->height()/(2*zoom));
    double r1 = (double) (outImage.height()) - r0;

    if(r0 < r1)
    {
        yScroll = min(r1, max(r0, yScroll));
        double newScrollY = (yScroll - r0)/(r1 - r0);
        newScrollY *= (double) ui->verticalScrollBar->maximum();

        zoomChanged = true;
        ui->verticalScrollBar->setVisible(true);
        ui->verticalScrollBar->setValue((int) newScrollY);
    }
    else
    {
        yScroll = (double) outImage.height()/2.0;
        ui->verticalScrollBar->setVisible(false);
    }

    DrawDisplayImage();
}

void MainWindow::Brightness(int val)
{
    DrawDisplayImage();
}

void MainWindow::Scroll(int val)
{
    if(zoomChanged == false)
    {
        int zoom = ui->zoomSlider->value()/20 + 1;
        double c0 = (double) (ui->ImgDisplay->width()/(2*zoom));
        double c1 = (double) (outImage.width()) - c0;

        if(c0 < c1)
        {
            double del = (double)  ui->horizontalScrollBar->value()/ (double) ui->horizontalScrollBar->maximum();
            xScroll = del*(c1 - c0) + c0;
        }
        else
            xScroll = (double) outImage.width()/2.0;

        double r0 = (double) (ui->ImgDisplay->height()/(2*zoom));
        double r1 = (double) (outImage.height()) - r0;

        if(r0 < r1)
        {
            double del = (double)  ui->verticalScrollBar->value()/ (double) ui->verticalScrollBar->maximum();
            yScroll = del*(r1 - r0) + r0;
        }
        else
            yScroll = (double) outImage.height()/2.0;
    }

    zoomChanged = false;

    DrawDisplayImage();
}

void MainWindow::OpenImage()
{
    const QString title;

    QString fileName = QFileDialog::getOpenFileName(this, title);

    if (!fileName.isEmpty())
        inImage.load(fileName);

    outImage = inImage.copy();
    xScroll = (double) outImage.width()/2.0;
    yScroll = (double) outImage.height()/2.0;
    ui->zoomSlider->setValue(0);

    DrawDisplayImage();

}

void MainWindow::SaveImage()
{
    const QString title;

    QString fileName = QFileDialog::getSaveFileName(this, title);

    if (!fileName.isEmpty())
        outImage.save(fileName);
}

void MainWindow::SaveDisplayImage()
{
    const QString title;

    QString fileName = QFileDialog::getSaveFileName(this, title);

    if (!fileName.isEmpty())
        displayImage.save(fileName);
}

void MainWindow::ResetImage()
{
    int w = outImage.width();

    outImage = inImage.copy();

    if(w != outImage.width())
    {
        xScroll = (double) outImage.width()/2.0;
        yScroll = (double) outImage.height()/2.0;
        ui->zoomSlider->setValue(1);
    }

    DrawDisplayImage();
}

void MainWindow::ToggleImage()
{
    DrawDisplayImage();
}

void MainWindow::AddNoise()
{
    double mag = ui->noiseSpinBox->value();

    AddNoise(&outImage, mag, ui->colorNoiseCheckBox->isChecked());

    DrawDisplayImage();
}


void MainWindow::MeanBlurImage()
{
    int radius = ui->meanBox->value();

    MeanBlurImage(&outImage, radius);

    DrawDisplayImage();
}

void MainWindow::MedianImage()
{
    int radius = ui->meanBox->value();

    ConvertQImage2Double(outImage);
    MedianImage(Image, radius);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::GaussianBlurImage()
{
    double sigma = ui->blurSpinBox->value();

    ConvertQImage2Double(outImage);
    if(ui->separableCheckBox->isChecked())
        SeparableGaussianBlurImage(Image, sigma);
    else
        GaussianBlurImage(Image, sigma);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::FirstDerivImage_x()
{
    double sigma = ui->firstDerivSpinBox->value();

    ConvertQImage2Double(outImage);
    FirstDerivImage_x(Image, sigma);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::FirstDerivImage_y()
{
    double sigma = ui->firstDerivSpinBox->value();

    ConvertQImage2Double(outImage);
    FirstDerivImage_y(Image, sigma);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::SecondDerivImage()
{
    double sigma = ui->secondDerivSpinBox->value();

    ConvertQImage2Double(outImage);
    SecondDerivImage(Image, sigma);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::SharpenImage()
{
    double sigma = ui->sharpenSigmaSpinBox->value();
    double mag = ui->sharpenMagSpinBox->value();

    ConvertQImage2Double(outImage);
    SharpenImage(Image, sigma, mag);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::RotateImage()
{
    double orien = ui->orientationSpinBox->value();

    ConvertQImage2Double(outImage);
    RotateImage(Image, orien);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::SobelImage()
{
    BlackWhiteImage(&outImage);
    ConvertQImage2Double(outImage);
    SobelImage(Image);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::BlackWhiteImage()
{
    BlackWhiteImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::FindPeaksImage()
{
    double thres = ui->peakThresholdSpinBox->value();

    BlackWhiteImage(&outImage);
    ConvertQImage2Double(outImage);
    FindPeaksImage(Image, thres);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::HoughImage()
{
    ConvertQImage2Double(outImage);
    HoughImage(Image);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::SmartKMeans()
{
    ConvertQImage2Double(outImage);
    SmartKMeans(Image);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}

void MainWindow::RandomSeedImage()
{
    int num_clusters = ui->clusterBox->value();

    ConvertQImage2Double(outImage);
    RandomSeedImage(Image, num_clusters);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();

}
void MainWindow::PixelSeedImage()
{
     int num_clusters = ui->clusterBox->value();

     ConvertQImage2Double(outImage);
     PixelSeedImage(Image, num_clusters);
     ConvertDouble2QImage(&outImage);

     DrawDisplayImage();
}
void MainWindow::HistogramSeedImage()
{
    int num_clusters = ui->clusterBox->value();

    ConvertQImage2Double(outImage);
    HistogramSeedImage(Image, num_clusters);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}
void MainWindow::HalfImage()
{
    HalfImage(outImage);

    xScroll = (double) outImage.width()/2.0;
    yScroll = (double) outImage.height()/2.0;
    ui->zoomSlider->setValue(1);

    DrawDisplayImage();
}

void MainWindow::BilateralImage()
{
    double sigmaS = ui->bilateralSigmaSSpinBox->value();
    double sigmaI = ui->bilateralSigmaISpinBox->value();

    ConvertQImage2Double(outImage);
    BilateralImage(Image, sigmaS, sigmaI);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();
}
