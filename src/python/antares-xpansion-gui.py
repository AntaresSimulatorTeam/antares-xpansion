from PyQt5.QtGui import QFont, QMovie, QIcon
from PyQt5.QtWidgets import QApplication, QLabel, QLineEdit, QPushButton, QVBoxLayout, \
    QHBoxLayout, QWidget, QFileDialog, QRadioButton, QSpacerItem, QSizePolicy, QPlainTextEdit, QMessageBox
from PyQt5.QtCore import QProcess, QByteArray
from pathlib import Path

import resources


class MainWidget(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent=parent)

        self.setWindowTitle('antares-xpansion-launcher-gui')

        self.mainLayout = QVBoxLayout()

        self._initInstallDirSelectionWidget()
        self._initAntaresStudySelectionWidget()
        self._initAntaresXpansionRunWidget()
        self._initLogWidget()

        self.setLayout(self.mainLayout)

        self._check_run_availability()

    def _initLogWidget(self):
        logLayout = QHBoxLayout()
        self.logTextEdit = QPlainTextEdit()
        self.logTextEdit.setReadOnly(True)
        font = QFont()
        font.setFamily(u"DejaVu Sans Mono")
        self.logTextEdit.setFont(font)
        logLayout.addWidget(self.logTextEdit)
        self.mainLayout.addLayout(logLayout)

    def _initAntaresXpansionRunWidget(self):
        methodLayout = QHBoxLayout()

        self.mpibendersRadioButton = QRadioButton('mpibenders')
        methodLayout.addWidget(self.mpibendersRadioButton)
        self.sequentialRadioButton = QRadioButton('sequential')
        self.sequentialRadioButton.setChecked(True)
        methodLayout.addWidget(self.sequentialRadioButton)

        methodLayout.addSpacerItem(QSpacerItem(1, 1, QSizePolicy.Expanding, QSizePolicy.Fixed))

        self.runningLabel = QLabel()
        self.movie = QMovie(":/images/loading.gif", QByteArray())
        self.runningLabel.setMovie(self.movie)
        self.movie.start()
        self.runningLabel.setVisible(False)
        methodLayout.addWidget(self.runningLabel)

        self.p = QProcess()
        self.p.readyReadStandardOutput.connect(self.handle_stdout)
        self.p.readyReadStandardError.connect(self.handle_stderr)
        self.p.stateChanged.connect(self.handle_state)
        self.p.finished.connect(self.cleanup_process)

        self.runButton = QPushButton('Run')
        self._set_run_label()
        self.runButton.clicked.connect(self.run_or_stop)
        methodLayout.addWidget(self.runButton)

        self.mainLayout.addLayout(methodLayout)

    def _initAntaresStudySelectionWidget(self):
        layoutStudyPath = QHBoxLayout()
        layoutStudyPath.addWidget(QLabel('Antares study path'))
        self.studyPathTextEdit = QLineEdit()
        layoutStudyPath.addWidget(self.studyPathTextEdit)
        selectButton = QPushButton('...')
        selectButton.clicked.connect(self.select_study_path)
        layoutStudyPath.addWidget(selectButton)
        self.mainLayout.addLayout(layoutStudyPath)

    def _initInstallDirSelectionWidget(self):
        layoutInstallDir = QHBoxLayout()
        layoutInstallDir.addWidget(QLabel('AntaresXpansion install directory'))
        self.installDirTextEdit = QLineEdit('../bin')
        layoutInstallDir.addWidget(self.installDirTextEdit)
        selectInstallDirButton = QPushButton('...')
        selectInstallDirButton.clicked.connect(self.select_install_dir)
        layoutInstallDir.addWidget(selectInstallDirButton)
        self.mainLayout.addLayout(layoutInstallDir)

    def set_study_path(self, study_path: str):
        self.studyPathTextEdit.setText(study_path)
        self._check_run_availability()

    def set_install_dir(self, install_dir: str):
        self.installDirTextEdit.setText(install_dir)
        self._check_run_availability()

    def select_study_path(self):
        self.set_study_path(
            QFileDialog.getExistingDirectory(self, 'Select study folder', self.studyPathTextEdit.text()))

    def select_install_dir(self):
        self.set_install_dir(
            QFileDialog.getExistingDirectory(self, 'Select install folder', self.installDirTextEdit.text()))

    def _check_run_availability(self):
        install_dir = self.installDirTextEdit.text()
        study_path = self.studyPathTextEdit.text()
        self.runButton.setEnabled(len(install_dir) and len(study_path))

    def _get_method(self):
        if self.mpibendersRadioButton.isChecked():
            return self.mpibendersRadioButton.text()
        if self.sequentialRadioButton.isChecked():
            return self.sequentialRadioButton.text()

    def handle_stdout(self):
        data = self.p.readAllStandardOutput()
        stdout = bytes(data).decode("utf8")
        self.message(stdout)

    def handle_stderr(self):
        data = self.p.readAllStandardError()
        stderr = bytes(data).decode("utf8")
        self.message(stderr)

    def handle_state(self, state):
        if state == QProcess.NotRunning:
            self._set_run_label()
        else:
            self._set_stop_label()

    def _set_stop_label(self):
        self.runButton.setText("Stop")
        self.runButton.setIcon(QIcon(":/images/stop-48.png"))
        self.runningLabel.setVisible(True)

    def _set_run_label(self):
        self.runButton.setText("Run")
        self.runButton.setIcon(QIcon(":/images/play-48.png"))
        self.runningLabel.setVisible(False)

    def message(self, s):
        self.logTextEdit.appendPlainText(s.rstrip('\r\n'))

    def cleanup_process(self):
        self.p.close();

    def run_or_stop(self):
        if self.p and self.p.isOpen():
            qm = QMessageBox()
            ret = qm.question(self, "Stop simulation", "Do you want to stop current simulation ?")
            if ret == qm.Yes:
                self.p.close()
        else:
            self.run()

    def run(self):
        self.logTextEdit.clear()
        study_path = self.studyPathTextEdit.text()
        install_dir = self.installDirTextEdit.text()
        install_dir_full = str(Path(install_dir).resolve())
        method = self._get_method()
        self.p.start("python", ["launch.py", "--installDir", install_dir_full, "--dataDir",
                                str(study_path), "--method", method, "--step", "full", "-n", "2"])
        self._set_stop_label()

app = QApplication([])

window = MainWidget()
window.show()
app.exec()
