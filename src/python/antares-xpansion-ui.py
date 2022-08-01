import os
import sys
from pathlib import Path

import psutil
import yaml
from PyQt5.QtCore import QProcess, QByteArray, QSettings
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont, QMovie, QIcon
from PyQt5.QtWidgets import QApplication, QLabel, QLineEdit, QPushButton, QVBoxLayout, \
    QHBoxLayout, QWidget, QFileDialog, QRadioButton, QSpacerItem, QSizePolicy, QPlainTextEdit, QMessageBox, QGridLayout, \
    QComboBox, QGroupBox, QSpinBox, QCheckBox

BENDERS_STEP = "benders"
STEPS = ["full", "antares", "problem_generation",
         BENDERS_STEP, "study_update", "sensitivity", "resume"]
STEP_WITH_SIMULATION_NAME = ["problem_generation",
                             "benders", "study_update", "sensitivity", "resume"]
NEW_SIMULATION_NAME = "New"
LAST_ANTARES_STUDY_DIR = "last_antares_study_dir"


class MainWidget(QWidget):

    def __init__(self, parent=None):
        QWidget.__init__(self, parent=parent)

        self.setWindowTitle('antares-xpansion-ui')

        self._define_install_dir()

        self._settings = QSettings("pyqt_settings.ini", QSettings.IniFormat)

        self._define_launcher_name()
        self._main_layout = QVBoxLayout()

        self._init_antares_study_selection_widget()
        self._init_antares_xpansion_run_widget()
        self._init_log_widget()

        self.setLayout(self._main_layout)

        self._check_run_availability()

    def _define_install_dir(self):
        self._install_dir = None

        if Path('config-ui.yaml').is_file():
            with open('config-ui.yaml') as file:
                content = yaml.full_load(file)
                if content is not None:
                    self._install_dir = content.get('INSTALL_DIR', "bin")

    def _define_launcher_name(self):
        self._launcher_name = "antares-xpansion-launcher"
        if sys.platform.startswith("win32"):
            self._launcher_name += ".exe"

    def _init_log_widget(self):
        log_layout = QHBoxLayout()
        self._log_text_edit = QPlainTextEdit()
        self._log_text_edit.setReadOnly(True)
        font = QFont()
        font.setFamily("DejaVu Sans Mono")
        self._log_text_edit.setFont(font)
        log_layout.addWidget(self._log_text_edit)
        self._main_layout.addLayout(log_layout)

    def _init_antares_xpansion_run_widget(self):

        self._xpansion_config_layout = QHBoxLayout()

        self._init_step_selection_widget()
        self._init_xpansion_config_widget()

        self._xpansion_config_layout.addSpacerItem(
            QSpacerItem(1, 1, QSizePolicy.Expanding, QSizePolicy.Fixed))
        self._init_xpansion_run_widget()

        self._main_layout.addLayout(self._xpansion_config_layout)

    def _init_xpansion_config_widget(self):

        method_layout = QHBoxLayout()
        self._sequential_radio_button = QRadioButton('Sequential')
        self._sequential_radio_button.setChecked(True)
        self._sequential_radio_button.toggled.connect(self._method_changed)
        method_layout.addWidget(self._sequential_radio_button)
        self._mpibenders_radio_button = QRadioButton('Parallel')
        self._mpibenders_radio_button.toggled.connect(self._method_changed)
        method_layout.addWidget(self._mpibenders_radio_button)
        method_layout.addWidget(QLabel("core number"))
        self._nb_core_edit = QSpinBox()
        self._nb_core_edit.setMinimum(2)
        self._nb_core_edit.setMaximum(128)
        cpu_count = psutil.cpu_count(logical=False)
        self._nb_core_edit.setValue(cpu_count)
        self._nb_core_edit.setEnabled(False)
        method_layout.addWidget(self._nb_core_edit)
        nb_cpu_label = QLabel(
            "<a href=\"{nb_cpu}\"><span style=\"text-decoration: none;\">available physical cores {nb_cpu}</span></a>".format(
                nb_cpu=cpu_count))
        nb_cpu_label.setTextInteractionFlags(Qt.LinksAccessibleByMouse)
        method_layout.addWidget(nb_cpu_label)
        method_gb = QGroupBox("Method")
        method_gb.setLayout(method_layout)
        self._xpansion_config_layout.addWidget(method_gb)

        option_gb = QGroupBox("Options")
        option_layout = QVBoxLayout()
        self._keep_mps_checkbox = QCheckBox("Keep intermediate files")
        self._keep_mps_checkbox.setChecked(False)
        option_layout.addWidget(self._keep_mps_checkbox)
        self._initialize_problems_startup = QCheckBox("Initialize problems at startup")
        self._initialize_problems_startup.setChecked(True)
        option_layout.addWidget(self._initialize_problems_startup)
        option_gb.setLayout(option_layout)
        self._xpansion_config_layout.addWidget(option_gb)

        #Conect at the end to avoid cyclical dependencies on widget existence
        nb_cpu_label.linkActivated.connect(self._use_available_core)
        for step in STEPS:
            self._step_buttons[step].toggled.connect(self._step_changed)
        self._step_buttons["full"].setChecked(True)

    def _init_xpansion_run_widget(self):
        self._running_label = QLabel()
        self.movie = QMovie(":/images/loading.gif", QByteArray())
        self._running_label.setMovie(self.movie)
        self.movie.start()
        self._running_label.setVisible(False)
        self._xpansion_config_layout.addWidget(self._running_label)
        self._run_process = QProcess()
        self._run_process.readyReadStandardOutput.connect(self._handle_stdout)
        self._run_process.readyReadStandardError.connect(self._handle_stderr)
        self._run_process.stateChanged.connect(self._handle_state)
        self._run_process.finished.connect(self._cleanup_run_process)
        self._run_button = QPushButton('Run')
        self._set_run_label()
        self._run_button.clicked.connect(self._run_or_stop)
        self._xpansion_config_layout.addWidget(self._run_button)

    def _init_antares_study_selection_widget(self):
        layout_study_path = QGridLayout()

        layout_study_path.addWidget(QLabel('Antares study path'), 0, 0)
        self._study_path_text_edit = QLineEdit(
            self._settings.value(LAST_ANTARES_STUDY_DIR))
        layout_study_path.addWidget(self._study_path_text_edit, 0, 1)
        select_button = QPushButton('...')
        select_button.clicked.connect(self._select_study_path)
        layout_study_path.addWidget(select_button, 0, 2)

        self._combo_simulation_name = QComboBox()
        self._combo_simulation_name.currentTextChanged.connect(
            self._simulation_name_changed)
        layout_study_path.addWidget(QLabel('Simulation name'), 1, 0)
        layout_study_path.addWidget(self._combo_simulation_name, 1, 1)
        self._init_simulation_name_combo(
            self._settings.value(LAST_ANTARES_STUDY_DIR))

        self._main_layout.addLayout(layout_study_path)

    def _init_step_selection_widget(self):
        step_layout = QHBoxLayout()

        self._step_buttons = {}
        for step in STEPS:
            self._step_buttons[step] = QRadioButton(step)
            self._step_buttons[step].setEnabled(
                step not in STEP_WITH_SIMULATION_NAME)
            step_layout.addWidget(self._step_buttons[step])

        self._step_gb = QGroupBox("Steps")
        self._step_gb.setLayout(step_layout)
        self._xpansion_config_layout.addWidget(self._step_gb)

    def set_study_path(self, study_path: str):
        self._settings.setValue(LAST_ANTARES_STUDY_DIR, study_path)
        self._study_path_text_edit.setText(study_path)
        self._init_simulation_name_combo(study_path)
        self._check_run_availability()

    def _init_simulation_name_combo(self, study_path: str):
        if study_path:
            self._combo_simulation_name.blockSignals(True)
            self._combo_simulation_name.clear()
            self._combo_simulation_name.addItem(NEW_SIMULATION_NAME)
            self._insert_study_simulation_in_combobox(study_path)
            self._combo_simulation_name.blockSignals(False)

    def _insert_study_simulation_in_combobox(self, study_path):
        output_path = Path(study_path) / 'output'
        if output_path.is_dir():
            for directory in sorted(output_path.iterdir(), key=os.path.getmtime, reverse=True):
                if (output_path / directory).is_dir():
                    self._combo_simulation_name.addItem(directory.name)

    def _select_study_path(self):
        self.set_study_path(
            QFileDialog.getExistingDirectory(self, 'Select study folder', self._study_path_text_edit.text()))

    def _check_run_availability(self):
        study_path = self._study_path_text_edit.text()
        run_available = Path(study_path).is_dir()
        self._run_button.setEnabled(run_available)
        self._step_gb.setEnabled(run_available)

    def _get_method(self):
        if self._mpibenders_radio_button.isChecked():
            return "mpibenders"
        if self._sequential_radio_button.isChecked():
            return "sequential"

    def _get_step(self):
        for step in self._step_buttons:
            if self._step_buttons[step].isChecked():
                return step

    def _get_nb_core(self):
        return self._nb_core_edit.value()

    def _get_keep_mps(self):
        return self._keep_mps_checkbox.isChecked()

    def _handle_stdout(self):
        data = self._run_process.readAllStandardOutput()
        stdout = bytes(data).decode("utf8")
        self._add_text_to_log(stdout)

    def _handle_stderr(self):
        data = self._run_process.readAllStandardError()
        stderr = bytes(data).decode("utf8")
        self._add_text_to_log(stderr)

    def _handle_state(self, state):
        if state == QProcess.NotRunning:
            self._set_run_label()
            if self._get_step() not in STEP_WITH_SIMULATION_NAME:
                self._init_simulation_name_combo(
                    self._study_path_text_edit.text())
        else:
            self._set_stop_label()

    def _step_changed(self):
        self._initialize_problems_startup.setEnabled(self._get_step() == BENDERS_STEP)

    def _method_changed(self):
        self._nb_core_edit.setEnabled(
            self._mpibenders_radio_button.isChecked())

    def _use_available_core(self):
        cpu_count = psutil.cpu_count(logical=False)
        self._nb_core_edit.setValue(cpu_count)

    def _set_stop_label(self):
        self._run_button.setText("Stop")
        self._run_button.setIcon(QIcon(":/images/stop-48.png"))
        self._running_label.setVisible(True)

    def _set_run_label(self):
        self._run_button.setText("Run")
        self._run_button.setIcon(QIcon(":/images/play-48.png"))
        self._running_label.setVisible(False)

    def _simulation_name_changed(self, text):
        for step in self._step_buttons:
            if text == NEW_SIMULATION_NAME:
                self._step_buttons[step].setEnabled(
                    step not in STEP_WITH_SIMULATION_NAME)
            else:
                self._step_buttons[step].setEnabled(
                    step in STEP_WITH_SIMULATION_NAME)

    def _add_text_to_log(self, s):
        self._log_text_edit.appendPlainText(s.rstrip('\r\n'))

    def _cleanup_run_process(self):
        self._run_process.close()

    def _run_or_stop(self):
        if self._run_process and self._run_process.isOpen():
            qm = QMessageBox()
            ret = qm.question(self, "Stop simulation",
                              "Do you want to stop current simulation ?")
            if ret == qm.Yes:
                self._run_process.close()
        else:
            self._run()

    def _run(self):
        self._log_text_edit.clear()
        study_path = self._study_path_text_edit.text()
        if study_path:
            if Path(study_path).is_dir():
                self._run_study(study_path)
            else:
                self._add_text_to_log("Study path is not a directory")
        else:
            self._add_text_to_log("No study path defined")

    def _run_study(self, study_path):
        commands = [
            "--dataDir", str(study_path),
            "--method", self._get_method(),
            "--step", self._get_step(),
            "-n", str(self._get_nb_core())]
        program_in_python_package = None
        if self._install_dir is not None and Path(self._install_dir).is_dir():
            install_dir_full = str(Path(self._install_dir).resolve())

            program_in_python_package = Path(os.path.abspath(
                __file__)).parent / self._launcher_name
            commands.extend(["--installDir", install_dir_full])

        program_in_single_package = Path(
            sys.executable).parent / self._launcher_name

        if self._get_keep_mps():
            commands.append("--keepMps")
        if not self._step_buttons["full"].isChecked():
            commands.append("--simulationName")
            commands.append(self._combo_simulation_name.currentText())
        if self._get_step() == BENDERS_STEP:
            commands.extend(["--construct_all_problems", str(self._initialize_problems_startup.isChecked())])
        if Path("launch.py").is_file():
            commands.insert(0, "launch.py")
            program = str(Path(sys.executable))
        elif program_in_python_package is not None and program_in_python_package.is_dir():
            program = str(program_in_python_package)
        else:
            program = str(program_in_single_package)
        self._run_process.start(program, commands)
        self._set_stop_label()


app = QApplication([])

window = MainWidget()
window.show()
app.exec()
