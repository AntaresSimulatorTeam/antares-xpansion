from PyQt5.QtWidgets import QTextEdit, QApplication, QLabel, QLineEdit, QComboBox, QPushButton, QVBoxLayout , QHBoxLayout, QWidget, QFileDialog, QRadioButton, QSpacerItem, QSizePolicy
from PyQt5.QtCore import  Qt, QThread
from pathlib import Path

import sys
import subprocess

class MainWidget(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent=parent)

        self.setWindowTitle('antares-xpansion-launcher-gui')
        
        mainLayout = QVBoxLayout()

        layoutInstallDir = QHBoxLayout()
        layoutInstallDir.addWidget(QLabel('AntaresXpansion install directory'))
        self.installDirTextEdit = QLineEdit('../bin')
        layoutInstallDir.addWidget(self.installDirTextEdit)
        selectInstallDirButton = QPushButton('Select')
        selectInstallDirButton.clicked.connect(self.select_install_dir)
        layoutInstallDir.addWidget(selectInstallDirButton)

        mainLayout.addLayout(layoutInstallDir)
        
        layoutStudyPath = QHBoxLayout()
        layoutStudyPath.addWidget(QLabel('Antares study path'))
        self.studyPathTextEdit = QLineEdit()
        layoutStudyPath.addWidget(self.studyPathTextEdit)
        selectButton = QPushButton('Select')
        selectButton.clicked.connect(self.select_study_path)
        layoutStudyPath.addWidget(selectButton)
        
        mainLayout.addLayout(layoutStudyPath)
        
        methodLayout = QHBoxLayout()
        self.mpibendersRadioButton = QRadioButton('mpibenders')
        methodLayout.addWidget(self.mpibendersRadioButton)
        self.sequentialRadioButton = QRadioButton('sequential')
        self.sequentialRadioButton.setChecked(True)
        methodLayout.addWidget(self.sequentialRadioButton)
        methodLayout.addSpacerItem(QSpacerItem(1,1, QSizePolicy.Expanding, QSizePolicy.Fixed))
        self.runButton = QPushButton('Run')
        self.runButton.clicked.connect(self.run)
        methodLayout.addWidget(self.runButton)
        
        mainLayout.addLayout(methodLayout)

        logLayout = QHBoxLayout()
        self.logTextEdit = QTextEdit()
        self.logTextEdit.setReadOnly(True)
        logLayout.addWidget(self.logTextEdit)

        mainLayout.addLayout(logLayout)
        
        self.setLayout(mainLayout)

        self._check_run_availability()
        
    def set_study_path(self, study_path : str):
        self.studyPathTextEdit.setText(study_path)
        self._check_run_availability()

    def set_install_dir(self, install_dir : str):
        self.installDirTextEdit.setText(install_dir)
        self._check_run_availability()

    def select_study_path(self):
        self.set_study_path(QFileDialog.getExistingDirectory(self, 'Select study folder', self.studyPathTextEdit.text()))

    def select_install_dir(self):
        self.set_install_dir(QFileDialog.getExistingDirectory(self, 'Select install folder', self.installDirTextEdit.text()))

    def _check_run_availability(self):
        install_dir = self.installDirTextEdit.text()
        study_path = self.studyPathTextEdit.text()
        self.runButton.setEnabled(len(install_dir) and len(study_path))

    def _get_method(self):
        if self.mpibendersRadioButton.isChecked():
            return self.mpibendersRadioButton.text()
        if self.sequentialRadioButton.isChecked():
            return self.sequentialRadioButton.text()
        
    def run(self):

        QApplication.setOverrideCursor(Qt.WaitCursor)

        self.logTextEdit.clear()
        study_path = self.studyPathTextEdit.text()
        install_dir = self.installDirTextEdit.text()

        install_dir_full = str(Path(install_dir).resolve())
        method = self._get_method()

        command = [sys.executable, "launch.py", "--installDir", install_dir_full, "--dataDir",
                   str(study_path), "--method", method, "--step", "full", "-n", "2"]

        process = subprocess.Popen(command, stdout=subprocess.PIPE)
        while process.poll() is None:
            QThread.msleep(100)
            QApplication.processEvents()
            output = process.stdout.readline()
            if output:
                self.logTextEdit.append(str(output.decode('UTF-8')).rstrip('\r\n'))
                print (output)
        rc = process.poll()

        QApplication.restoreOverrideCursor()
    
app = QApplication([])

window = MainWidget()
window.show()
app.exec()