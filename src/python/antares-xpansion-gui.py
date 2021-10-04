import os

from PyQt5.QtGui import QFont, QMovie, QIcon
from PyQt5.QtWidgets import QApplication, QLabel, QLineEdit, QPushButton, QVBoxLayout, \
    QHBoxLayout, QWidget, QFileDialog, QRadioButton, QSpacerItem, QSizePolicy, QPlainTextEdit, QMessageBox, QGridLayout, \
    QComboBox, QGroupBox
from PyQt5.QtCore import QProcess, QByteArray
from pathlib import Path

import resources

STEP_WITH_SIMULATION_NAME = ["getnames", "lp", "optim", "update"]
NEW_SIMULATION_NAME = "New"

class MainWidget(QWidget):

    def __init__(self, parent=None):
        QWidget.__init__(self, parent=parent)

        self.setWindowTitle('antares-xpansion-launcher-gui')

        self.mainLayout = QVBoxLayout()

        self._initInstallDirSelectionWidget()
        self._initAntaresStudySelectionWidget()
        self._initAntaresXpansionRunWidget()
        self._initStepSelectionWidget()
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

        runLayout = QHBoxLayout()
        methodLayout = QHBoxLayout()

        self.mpibendersRadioButton = QRadioButton('mpibenders')
        methodLayout.addWidget(self.mpibendersRadioButton)
        self.sequentialRadioButton = QRadioButton('sequential')
        self.sequentialRadioButton.setChecked(True)
        methodLayout.addWidget(self.sequentialRadioButton)

        methodLayout.addSpacerItem(QSpacerItem(1, 1, QSizePolicy.Expanding, QSizePolicy.Fixed))
        runLayout.addLayout(methodLayout)

        self.runningLabel = QLabel()
        self.movie = QMovie(":/images/loading.gif", QByteArray())
        self.runningLabel.setMovie(self.movie)
        self.movie.start()
        self.runningLabel.setVisible(False)
        runLayout.addWidget(self.runningLabel)

        self.p = QProcess()
        self.p.readyReadStandardOutput.connect(self.handle_stdout)
        self.p.readyReadStandardError.connect(self.handle_stderr)
        self.p.stateChanged.connect(self.handle_state)
        self.p.finished.connect(self.cleanup_process)

        self.runButton = QPushButton('Run')
        self._set_run_label()
        self.runButton.clicked.connect(self.run_or_stop)
        runLayout.addWidget(self.runButton)

        self.mainLayout.addLayout(runLayout)

    def _initAntaresStudySelectionWidget(self):
        layoutStudyPath = QGridLayout()

        layoutStudyPath.addWidget(QLabel('Antares study path'), 0, 0)
        self.studyPathTextEdit = QLineEdit()
        layoutStudyPath.addWidget(self.studyPathTextEdit, 0, 1)
        selectButton = QPushButton('...')
        selectButton.clicked.connect(self.select_study_path)
        layoutStudyPath.addWidget(selectButton, 0, 2)

        self.comboSimulationName = QComboBox()
        self.comboSimulationName.currentTextChanged.connect(self.simulation_name_changed)
        layoutStudyPath.addWidget(QLabel('Simulation name'), 1, 0)
        layoutStudyPath.addWidget(self.comboSimulationName, 1, 1)

        self.mainLayout.addLayout(layoutStudyPath)

    def _initStepSelectionWidget(self):
        stepLayout = QHBoxLayout()

        steps = ["full", "antares", "getnames", "lp", "optim", "update"]
        self.step_buttons = {}
        for step in steps:
            self.step_buttons[step] = QRadioButton(step)
            stepLayout.addWidget(self.step_buttons[step])

        self.step_buttons["full"].setChecked(True)

        stepLayout.addSpacerItem(QSpacerItem(1, 1, QSizePolicy.Expanding, QSizePolicy.Fixed))
        self.step_gb = QGroupBox("Steps")
        self.step_gb.setLayout(stepLayout)
        self.mainLayout.addWidget(self.step_gb)

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
        self._initSimulationNameCombo(study_path)
        self._check_run_availability()

    def _initSimulationNameCombo(self, study_path):
        output_path = Path(study_path) / 'output'
        self.comboSimulationName.clear()
        self.comboSimulationName.addItem(NEW_SIMULATION_NAME)
        for dir in sorted(output_path.iterdir(), key=os.path.getmtime, reverse=True):
            if (output_path / dir).is_dir():
                self.comboSimulationName.addItem(dir.name)

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
        run_available = len(install_dir) and len(study_path)
        self.runButton.setEnabled(run_available)
        self.step_gb.setEnabled(run_available)

    def _get_method(self):
        if self.mpibendersRadioButton.isChecked():
            return self.mpibendersRadioButton.text()
        if self.sequentialRadioButton.isChecked():
            return self.sequentialRadioButton.text()

    def _get_step(self):
        for step in self.step_buttons:
            if self.step_buttons[step].isChecked():
                return step

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
            if self._get_step() not in STEP_WITH_SIMULATION_NAME:
                self._initSimulationNameCombo(self.studyPathTextEdit.text())
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

    def simulation_name_changed(self, text):

        for step in self.step_buttons:
            if text == NEW_SIMULATION_NAME:
                self.step_buttons[step].setEnabled(step not in STEP_WITH_SIMULATION_NAME)
            else:
                self.step_buttons[step].setEnabled(step in STEP_WITH_SIMULATION_NAME)

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
        commands = ["launch.py", "--installDir", install_dir_full, "--dataDir",
                    str(study_path), "--method", self._get_method(), "--step", self._get_step(), "-n", "2"]

        if not self.step_buttons["full"].isChecked():
            commands.append("--simulationName")
            commands.append(self.comboSimulationName.currentText())

        self.p.start("python", commands)
        self._set_stop_label()


app = QApplication([])

window = MainWidget()
window.show()
app.exec()
