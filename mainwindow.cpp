#include "mainwindow.h"
#include "ui_mainwindow.h"


#ifdef Q_OS_MAC
    #define FILTER "(*.app)"
    #define PREDIRS "/Contents/Resources/Java"
    #define FONTSIZE 12
#else
    #define PREDIRS ""
    #ifdef Q_OS_WIN32
        #define FILTER "(*.exe)"
        #define FONTSIZE 10
    #else
        #error "OS not supported!
    #endif
#endif

const QString MainWindow::boards_txt_path = QString("/hardware/arduino/boards.txt");
const QString MainWindow::usbcore_cpp_path = QString("/hardware/arduino/cores/arduino/USBCore.cpp");
const QString MainWindow::caterina_path = QString("/hardware/arduino/bootloaders/caterina/");
const QString MainWindow::tunisuino_board = QString("\n##############################################################\n"
                                                      "\n"
                                                      "tunisuino.name=Tunisuino\n"
                                                      "tunisuino.upload.protocol=avr109\n"
                                                      "tunisuino.upload.maximum_size=28672\n"
                                                      "tunisuino.upload.speed=57600\n"
                                                      "tunisuino.upload.disable_flushing=true\n"
                                                      "tunisuino.bootloader.low_fuses=0xff\n"
                                                      "tunisuino.bootloader.high_fuses=0xd8\n"
                                                      "tunisuino.bootloader.extended_fuses=0xcb\n"
                                                      "tunisuino.bootloader.path=caterina\n"
                                                      "tunisuino.bootloader.file=Tunisuino.hex\n"
                                                      "tunisuino.bootloader.unlock_bits=0x3F\n"
                                                      "tunisuino.bootloader.lock_bits=0x2F\n"
                                                      "tunisuino.build.mcu=atmega32u4\n"
                                                      "tunisuino.build.f_cpu=16000000L\n"
                                                      "tunisuino.build.vid=0x1d50\n"
                                                      "tunisuino.build.pid=0x1920\n"
                                                      "tunisuino.build.core=arduino\n"
                                                      "tunisuino.build.variant=leonardo\n");
const QString MainWindow::tunisuino_iproduct = QString("#elif USB_PID == 0x1920\n"
                                                       "\t'T','u','n','i','s','u','i','n','o',' ',' ',' ',' ',' ',' ',' '\n");
const QString MainWindow::tunisuino_imanufacture = QString("#elif USB_VID == 0x1d50\n"
                                                           "\t'E','l','e','c',' ','A','r','t',' ',' ',' '\n");


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->install_button->setEnabled(false);
    ui->output_screen->setFontPointSize(FONTSIZE);
    print_info("Welcome to the Tunisuino Plugin Installer!\n\nPlease set the Arduino Ide path");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_path_button_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Arduino IDE Path"), "/", tr("Arduino IDE "FILTER));
    ui->path_lineedit->setText(fileName);
    if (!fileName.isEmpty()) {
        print_info("\nYou can now proceed with the install");
        ui->install_button->setEnabled(true);
    } else {
        ui->install_button->setEnabled(false);
    }
}

void MainWindow::on_install_button_clicked()
{
    QTextStream istream, ostream;
    print_info("\n");
    QString basepath = ui->path_lineedit->text();
#ifndef Q_OS_MAC
    basepath = basepath.left(basepath.lastIndexOf("/"));
#endif
    basepath.append(PREDIRS);


    /*
     *  Patch boards.txt
     */

    QString boards_txt_fullpath = basepath;
    boards_txt_fullpath.append(&boards_txt_path);
    QFile boards_txt(boards_txt_fullpath);
    if (!boards_txt.exists()) {
        print_warning(boards_txt_fullpath.append(" could not be found, please check the Arduino IDE path."));
        return;
    }
    if (!boards_txt.open(QIODevice::ReadOnly | QIODevice::Text)) {
        print_warning(boards_txt_fullpath.append(" could not be read, please check file permissions."));
        return;
    }
    istream.setDevice(&boards_txt);
    QString strfile = istream.readAll();
    boards_txt.close();
    if (strfile.contains("Tunisuino")) {
        print_info(boards_txt_fullpath.append(" is already patched."));
    } else {
        if (!boards_txt.open(QIODevice::WriteOnly | QIODevice::Text)) {
            print_warning(boards_txt_fullpath.append(" could not be modified, please check file permissions."));
            return;
        }
        ostream.setDevice(&boards_txt);
        strfile.insert(strfile.indexOf("###########################") - 1, tunisuino_board);
        ostream << strfile;
        boards_txt.close();
        print_info(boards_txt_fullpath.append(" has been patched successfully..."));
    }


    /*
     *  Patch USBCore.cpp
     */

    QString usbcore_cpp_fullpath = basepath;
    usbcore_cpp_fullpath.append(&usbcore_cpp_path);
    QFile usbcore_cpp(usbcore_cpp_fullpath);
    if (!usbcore_cpp.exists()) {
        print_warning(usbcore_cpp_fullpath.append(" could not be found, please check the Arduino IDE path."));
        return;
    }
    if (!usbcore_cpp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        print_warning(usbcore_cpp_fullpath.append(" could not be read, please check file permissions."));
        return;
    }
    istream.setDevice(&usbcore_cpp);
    strfile = istream.readAll();
    usbcore_cpp.close();
    if (strfile.contains("#elif USB_PID == 0x1920")) {
        print_info(usbcore_cpp_fullpath.append(" is already patched."));
    } else {
        int iproduct_pos = strfile.indexOf("const u16 STRING_IPRODUCT[17]");
        int imanufacture_pos = strfile.indexOf("const u16 STRING_IMANUFACTURER[12]");
        if (iproduct_pos == -1 || imanufacture_pos == -1) {
            print_warning(usbcore_cpp_fullpath.append(" could not be patched by this software version."));
            return;
        }

        if (!usbcore_cpp.open(QIODevice::WriteOnly | QIODevice::Text)) {
            print_warning(usbcore_cpp_fullpath.append(" could not be modified, please check file permissions."));
            return;
        }
        ostream.setDevice(&usbcore_cpp);

        strfile.insert(strfile.indexOf("#else", iproduct_pos), tunisuino_iproduct);
        imanufacture_pos = strfile.indexOf("const u16 STRING_IMANUFACTURER[12]");
        strfile.insert(strfile.indexOf("#else", imanufacture_pos), tunisuino_imanufacture);

        ostream << strfile;
        usbcore_cpp.close();
        print_info(usbcore_cpp_fullpath.append(" has been patched successfully..."));
    }


    /*
     *  Copy Tunisuino.hex to bootloaders
     */

    QString caterina_fullpath = basepath;
    caterina_fullpath.append(&caterina_path).append("Tunisuino.hex");
    QFile caterina(caterina_fullpath);
    if (caterina.exists()) {
        print_info(caterina_fullpath.append(" is already installed."));
    } else {
        if (!QFile::copy(":/Tunisuino.hex", caterina_fullpath)) {
            print_warning(caterina_fullpath.append(" could not be installed, please check file permissions."));
            return;
        } else {
            print_info(caterina_fullpath.append(" has been installed successfully..."));
        }
    }


#ifdef Q_OS_WIN32
    /*
     *  Copy Windows Driver
     */

    QString driver_fullpath = basepath;
    driver_fullpath.append("/drivers/Tunisuino.inf");
    QFile driver_inf(driver_fullpath);
    if (driver_inf.exists()) {
        print_info(driver_fullpath.append(" is already installed."));
    } else {
        if (!QFile::copy(":/Tunisuino.inf", driver_fullpath)) {
            print_warning(driver_fullpath.append(" could not be installed, please check file permissions."));
            return;
        } else {
            print_info(driver_fullpath.append(" has been installed successfully..."));
        }
    }
#endif

    print_warning("\nThe Tunisuino Plugin has been installed successfully!\n");
    ui->path_button->setEnabled(false);
    ui->path_lineedit->setEnabled(false);
    ui->install_button->setText("Done");
    connect(ui->install_button, SIGNAL(clicked()), this, SLOT(close()));
}

void MainWindow::print_info(QString str)
{
    ui->output_screen->append(str);
}

void MainWindow::print_warning(QString str)
{
    ui->output_screen->setFontWeight(QFont::DemiBold);
    ui->output_screen->setTextColor(QColor(255, 0, 0));
    ui->output_screen->append(str);
    ui->output_screen->setFontWeight(QFont::Normal);
    ui->output_screen->setTextColor(QColor(0, 0, 0));
}
