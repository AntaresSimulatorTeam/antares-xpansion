import os

import yaml
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont, QMovie, QIcon
from PyQt5.QtWidgets import QApplication, QLabel, QLineEdit, QPushButton, QVBoxLayout, \
    QHBoxLayout, QWidget, QFileDialog, QRadioButton, QSpacerItem, QSizePolicy, QPlainTextEdit, QMessageBox, QGridLayout, \
    QComboBox, QGroupBox, QSpinBox
from PyQt5.QtCore import QProcess, QByteArray, QSettings
from pathlib import Path

import resources

STEP_WITH_SIMULATION_NAME = ["getnames", "lp", "optim", "update"]
NEW_SIMULATION_NAME = "New"
INSTALL_DIR = "install_dir"
LAST_ANTARES_STUDY_DIR = "last_antares_study_dir"

class MainWidget(QWidget):

    def __init__(self, parent=None):
        QWidget.__init__(self, parent=parent)

        self.setWindowTitle('antares-xpansion-ui')

        self._define_install_dir()

        self.settings = QSettings("pyqt_settings.ini", QSettings.IniFormat)

        self.main_layout = QVBoxLayout()

        self._initAntaresStudySelectionWidget()
        self._initStepSelectionWidget()
        self._initAntaresXpansionRunWidget()
        self._initLogWidget()

        self.setLayout(self.main_layout)

        self._check_run_availability()

    def _define_install_dir(self):
        self.INSTALL_DIR = Path("bin")
        if Path('config-ui.yaml').is_file():
            with open('config-ui.yaml') as file:
                content = yaml.full_load(file)
                if content is not None:
                    self.INSTALL_DIR = content.get('INSTALL_DIR', "bin")

    def _initLogWidget(self):
        log_layout = QHBoxLayout()
        self.logTextEdit = QPlainTextEdit()
        self.logTextEdit.setReadOnly(True)
        font = QFont()
        font.setFamily("DejaVu Sans Mono")
        self.logTextEdit.setFont(font)
        log_layout.addWidget(self.logTextEdit)
        self.main_layout.addLayout(log_layout)

    def _initAntaresXpansionRunWidget(self):

        method_layout = QHBoxLayout()

        self.sequentialRadioButton = QRadioButton('Sequential')
        self.sequentialRadioButton.setChecked(True)
        self.sequentialRadioButton.toggled.connect(self._method_changed)
        method_layout.addWidget(self.sequentialRadioButton)

        self.mpibendersRadioButton = QRadioButton('Parallel')
        self.mpibendersRadioButton.toggled.connect(self._method_changed)

        method_layout.addWidget(self.mpibendersRadioButton)
        method_layout.addWidget(QLabel("core number"))
        self.nb_core_edit = QSpinBox()
        self.nb_core_edit.setMinimum(2)
        self.nb_core_edit.setMaximum(128)
        self.nb_core_edit.setValue(os.cpu_count())
        method_layout.addWidget(self.nb_core_edit)

        nb_cpu_label = QLabel("<a href=\"{nb_cpu}\"><span style=\"text-decoration: none;\">available core {nb_cpu}</span></a>".format(nb_cpu=os.cpu_count()))
        nb_cpu_label.setTextInteractionFlags(Qt.LinksAccessibleByMouse)
        nb_cpu_label.linkActivated.connect(self._use_available_core)
        method_layout.addWidget(nb_cpu_label)

        method_layout.addSpacerItem(QSpacerItem(1, 1, QSizePolicy.Expanding, QSizePolicy.Fixed))

        method_gb = QGroupBox("Method")
        method_gb.setLayout(method_layout)

        run_layout = QHBoxLayout()
        run_layout.addWidget(method_gb)

        self.runningLabel = QLabel()
        self.movie = QMovie(":/images/loading.gif", QByteArray())
        self.runningLabel.setMovie(self.movie)
        self.movie.start()
        self.runningLabel.setVisible(False)
        run_layout.addWidget(self.runningLabel)

        self.p = QProcess()
        self.p.readyReadStandardOutput.connect(self.handle_stdout)
        self.p.readyReadStandardError.connect(self.handle_stderr)
        self.p.stateChanged.connect(self.handle_state)
        self.p.finished.connect(self.cleanup_process)

        self.runButton = QPushButton('Run')
        self._set_run_label()
        self.runButton.clicked.connect(self.run_or_stop)
        run_layout.addWidget(self.runButton)

        self.main_layout.addLayout(run_layout)

    def _initAntaresStudySelectionWidget(self):
        layout_study_path = QGridLayout()

        layout_study_path.addWidget(QLabel('Antares study path'), 0, 0)
        self.studyPathTextEdit = QLineEdit(self.settings.value(LAST_ANTARES_STUDY_DIR))
        layout_study_path.addWidget(self.studyPathTextEdit, 0, 1)
        select_button = QPushButton('...')
        select_button.clicked.connect(self.select_study_path)
        layout_study_path.addWidget(select_button, 0, 2)

        self.comboSimulationName = QComboBox()
        self.comboSimulationName.currentTextChanged.connect(self.simulation_name_changed)
        layout_study_path.addWidget(QLabel('Simulation name'), 1, 0)
        layout_study_path.addWidget(self.comboSimulationName, 1, 1)
        self._initSimulationNameCombo(self.settings.value(LAST_ANTARES_STUDY_DIR))

        self.main_layout.addLayout(layout_study_path)

    def _initStepSelectionWidget(self):
        step_layout = QHBoxLayout()

        steps = ["full", "antares", "getnames", "lp", "optim", "update"]
        self.step_buttons = {}
        for step in steps:
            self.step_buttons[step] = QRadioButton(step)
            step_layout.addWidget(self.step_buttons[step])

        self.step_buttons["full"].setChecked(True)

        step_layout.addSpacerItem(QSpacerItem(1, 1, QSizePolicy.Expanding, QSizePolicy.Fixed))
        self.step_gb = QGroupBox("Steps")
        self.step_gb.setLayout(step_layout)
        self.main_layout.addWidget(self.step_gb)

    def set_study_path(self, study_path: str):
        self.settings.setValue(LAST_ANTARES_STUDY_DIR, study_path)
        self.studyPathTextEdit.setText(study_path)
        self._initSimulationNameCombo(study_path)
        self._check_run_availability()

    def _initSimulationNameCombo(self, study_path: str):
        if study_path:
            output_path = Path(study_path) / 'output'
            self.comboSimulationName.blockSignals(True)
            self.comboSimulationName.clear()
            self.comboSimulationName.addItem(NEW_SIMULATION_NAME)
            for dir in sorted(output_path.iterdir(), key=os.path.getmtime, reverse=True):
                if (output_path / dir).is_dir():
                    self.comboSimulationName.addItem(dir.name)
            self.comboSimulationName.blockSignals(False)

    def select_study_path(self):
        self.set_study_path(
            QFileDialog.getExistingDirectory(self, 'Select study folder', self.studyPathTextEdit.text()))

    def _check_run_availability(self):
        study_path = self.studyPathTextEdit.text()
        run_available = len(study_path)
        self.runButton.setEnabled(run_available)
        self.step_gb.setEnabled(run_available)

    def _get_method(self):
        if self.mpibendersRadioButton.isChecked():
            return "mpibenders"
        if self.sequentialRadioButton.isChecked():
            return "sequential"

    def _get_step(self):
        for step in self.step_buttons:
            if self.step_buttons[step].isChecked():
                return step

    def _get_nb_core(self):
        return self.nb_core_edit.value()

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

    def _method_changed(self):
        self.nb_core_edit.setEnabled(self.mpibendersRadioButton.isChecked())

    def _use_available_core(self):
        self.nb_core_edit.setValue(os.cpu_count())

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
        install_dir = self.INSTALL_DIR
        install_dir_full = str(Path(install_dir).resolve())

        program = "antares-xpansion-launcher.exe"
        commands = ["--installDir", install_dir_full,
                    "--dataDir", str(study_path),
                    "--method", self._get_method(),
                    "--step", self._get_step(),
                    "-n", str(self._get_nb_core())]

        if not self.step_buttons["full"].isChecked():
            commands.append("--simulationName")
            commands.append(self.comboSimulationName.currentText())

        if Path("launch.py").is_file():
            commands.insert(0, "launch.py")
            program = "python"

        self.p.start(program, commands)
        self._set_stop_label()


app = QApplication([])

window = MainWidget()
window.show()
app.exec()
