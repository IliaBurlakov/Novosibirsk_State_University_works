package minesweeper.view.gui;

import minesweeper.controller.GameController;
import minesweeper.model.*;
import minesweeper.view.GameSettings;
import minesweeper.view.View;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class GUI extends JFrame implements View {
    private GameModel model;
    private GameController controller;
    private JPanel gamePanel;
    private JLabel timeLabel;
    private JLabel flagsLabel;
    User user;
    private int cellSize = 30; //default size of cell 30x30

    public GUI() {
        GameSettings settings = showSettingsDialog();
        if (settings == null) System.exit(0);

        String nickname = JOptionPane.showInputDialog(
                this,
                "Enter your nickname:",
                "Nickname",
                JOptionPane.PLAIN_MESSAGE
        );

        if (nickname == null || nickname.trim().isEmpty()) {
            nickname = "Anonymous";
        }
        this.user = new User(nickname.trim());

        this.model = new GameModel(
                settings.getWidth(),
                settings.getHeight(),
                settings.getMines(),
                settings.getDifficulty()
        );
        this.controller = new GameController(model, this);
        this.controller.setUser(user);

        initializeUI();
        controller.startTimer();
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            GameSettings settings = new GameSettings();
            GameModel model = new GameModel(settings.getWidth(), settings.getHeight(), settings.getMines(), settings.getDifficulty());
            GameController controller = new GameController(model, null);
            GUI gui = new GUI();
            controller.setView(gui);
            gui.setVisible(true);
        });
    }

    @Override
    public void updateTime(int time) {
        timeLabel.setText("Time: " + time + " secs.");
    }

    @Override
    public void showMessage(String s) {
        JOptionPane.showMessageDialog(this, s, "New hight score!", JOptionPane.INFORMATION_MESSAGE);
    }

    private void initializeUI() {
        setTitle("Minesweeper by Ilia Burlakov");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout());
        setIconImage(getImage("icon.png"));

        JMenuBar menuBar = new JMenuBar();
        JMenu gameMenu = new JMenu("Game");
        JMenuItem newGameItem = new JMenuItem("New Game");
        JMenuItem exitItem = new JMenuItem("Exit");
        gameMenu.add(newGameItem);
        gameMenu.add(exitItem);

        JMenu helpMenu = new JMenu("Help");
        JMenuItem aboutItem = new JMenuItem("About");
        aboutItem.addActionListener(e -> showAboutDialog());
        helpMenu.add(aboutItem);
        menuBar.add(helpMenu);

        JMenuItem customGameItem = new JMenuItem("New Custom Game");
        gameMenu.add(customGameItem);

        menuBar.add(gameMenu);

        JMenuItem scoresItem = new JMenuItem("High Scores");
        scoresItem.addActionListener(e -> showHighScoresDialog());
        gameMenu.add(scoresItem);

        JMenu settingsMenu = new JMenu("Settings");
        menuBar.add(settingsMenu);

        JMenu scaleMenu = new JMenu("Scale");
        settingsMenu.add(scaleMenu);

        JRadioButtonMenuItem tinyItem = new JRadioButtonMenuItem("Tiny (10px)");
        JRadioButtonMenuItem xSmallItem = new JRadioButtonMenuItem("XSmall (15px)");
        JRadioButtonMenuItem smallItem = new JRadioButtonMenuItem("Small (20px)");
        JRadioButtonMenuItem mediumItem = new JRadioButtonMenuItem("Medium (30px)");
        JRadioButtonMenuItem largeItem = new JRadioButtonMenuItem("Large (40px)");
        JRadioButtonMenuItem xLargeItem = new JRadioButtonMenuItem("XLarge (50px)");
        JRadioButtonMenuItem xxLargeItem = new JRadioButtonMenuItem("XXLarge (75px)");
        JRadioButtonMenuItem xxxLargeItem = new JRadioButtonMenuItem("XXXLarge 100px)");

        ButtonGroup scaleGroup = new ButtonGroup();
        scaleGroup.add(tinyItem);
        scaleGroup.add(xSmallItem);
        scaleGroup.add(smallItem);
        scaleGroup.add(mediumItem);
        scaleGroup.add(largeItem);
        scaleGroup.add(xLargeItem);
        scaleGroup.add(xxLargeItem);
        scaleGroup.add(xxxLargeItem);

        scaleMenu.add(tinyItem);
        scaleMenu.add(xSmallItem);
        scaleMenu.add(smallItem);
        scaleMenu.add(mediumItem);
        scaleMenu.add(largeItem);
        scaleMenu.add(xLargeItem);
        scaleMenu.add(xxLargeItem);
        scaleMenu.add(xxxLargeItem);

        mediumItem.setSelected(true);

        tinyItem.addActionListener(e -> setScale(10));
        xSmallItem.addActionListener(e -> setScale(15));
        smallItem.addActionListener(e -> setScale(20));
        mediumItem.addActionListener(e -> setScale(30));
        largeItem.addActionListener(e -> setScale(40));
        xLargeItem.addActionListener(e -> setScale(50));
        xxLargeItem.addActionListener(e -> setScale(75));
        xxxLargeItem.addActionListener(e -> setScale(100));

        setJMenuBar(menuBar);


        customGameItem.addActionListener(e -> {
            GameSettings newSettings = showSettingsDialog();
            if (newSettings != null) {
                controller.stopTimer();
                this.model = new GameModel(newSettings.getWidth(), newSettings.getHeight(), newSettings.getMines(), newSettings.getDifficulty());
                this.controller = new GameController(model, this);
                rebuildGameField();
                controller.startTimer();
            }
        });

        JPanel infoPanel = new JPanel();
        timeLabel = new JLabel("Time: 0 secs.");
        flagsLabel = new JLabel("Flags: " + model.getFlagsLeft());
        infoPanel.add(timeLabel);
        infoPanel.add(flagsLabel);
        add(infoPanel, BorderLayout.NORTH);

        gamePanel = new JPanel(new GridLayout(model.getHeight(), model.getWidth(), 0, 0));
        rebuildGameField();
        JScrollPane scrollPane = new JScrollPane(gamePanel);
        scrollPane.getViewport().setPreferredSize(gamePanel.getPreferredSize()); // Примерный размер
        add(scrollPane, BorderLayout.CENTER);

        newGameItem.addActionListener(e -> controller.handleNewGame());
        exitItem.addActionListener(e -> System.exit(0));

        pack();
        setLocationRelativeTo(null);
    }

    private void setScale(int newSize) {
        this.cellSize = newSize;
        rebuildGameField();
        pack();
    }

    private void showAboutDialog() {
        String aboutText = "Minesweeper v1.0\nCreated by Ilia Burlakov\n2025";
        JOptionPane.showMessageDialog(
                this,
                aboutText,
                "About",
                JOptionPane.INFORMATION_MESSAGE
        );
    }

    private void showHighScoresDialog() {
        String[] difficulties = {"Easy", "Medium", "Hard"};
        String selectedDifficulty = (String) JOptionPane.showInputDialog(
                this,
                "Select difficulty:",
                "High Scores",
                JOptionPane.PLAIN_MESSAGE,
                null,
                difficulties,
                difficulties[0]
        );
        if (selectedDifficulty != null) {
            // Используем контроллер для получения рекордов
            String scores = controller.getHighScores(selectedDifficulty.toLowerCase());
            JOptionPane.showMessageDialog(
                    this,
                    scores,
                    "High Scores",
                    JOptionPane.INFORMATION_MESSAGE
            );
        }
    }


    private GameSettings showSettingsDialog() {
        // Создаем панель с компонентами
        JPanel panel = new JPanel(new GridLayout(4, 2));

        JComboBox<String> difficultyCombo = new JComboBox<>(new String[]{"Easy", "Medium", "Hard", "Custom"});
        JTextField widthField = new JTextField("9");
        JTextField heightField = new JTextField("9");
        JTextField minesPercentField = new JTextField("0.2");

        difficultyCombo.addActionListener(e -> {
            String selected = (String) difficultyCombo.getSelectedItem();
            boolean isCustom = "Custom".equals(selected);
            widthField.setEnabled(isCustom);
            heightField.setEnabled(isCustom);
            minesPercentField.setEnabled(isCustom);
        });

        panel.add(new JLabel("Difficulty:"));
        panel.add(difficultyCombo);
        panel.add(new JLabel("Width:"));
        panel.add(widthField);
        panel.add(new JLabel("Height:"));
        panel.add(heightField);
        panel.add(new JLabel("Mines (%):"));
        panel.add(minesPercentField);

        int result = JOptionPane.showConfirmDialog(
                this,
                panel,
                "Game Settings",
                JOptionPane.OK_CANCEL_OPTION
        );

        if (result == JOptionPane.OK_OPTION) {
            GameSettings settings = new GameSettings();
            String difficulty = (String) difficultyCombo.getSelectedItem();

            try {
                if ("Custom".equals(difficulty)) {
                    int width = Integer.parseInt(widthField.getText());
                    int height = Integer.parseInt(heightField.getText());
                    double minesPercent = Double.parseDouble(minesPercentField.getText());
                    settings.setCustom(width, height, minesPercent);
                } else {
                    settings.setDifficulty(difficulty.toLowerCase());
                }
                return settings;
            } catch (Exception e) {
                JOptionPane.showMessageDialog(this, "Invalid input!", "Error", JOptionPane.ERROR_MESSAGE);
                return null;
            }
        }
        return null;
    }

    private void rebuildGameField() {
        gamePanel.removeAll();
        gamePanel.setLayout(new GridLayout(model.getHeight(), model.getWidth(), 0, 0));
        gamePanel.setPreferredSize(new Dimension(
                model.getWidth() * cellSize,
                model.getHeight() * cellSize
        ));

        for (int y = 0; y < model.getHeight(); y++) {
            for (int x = 0; x < model.getWidth(); x++) {
                JButton cell = createCellButton(x, y);
                gamePanel.add(cell);
            }
        }

        gamePanel.revalidate();
        gamePanel.repaint();
    }

    private JButton createCellButton(int x, int y) {
        JButton button = new JButton();
        button.setPreferredSize(new Dimension(cellSize, cellSize));
        button.setMinimumSize(new Dimension(cellSize, cellSize));
        button.setMaximumSize(new Dimension(cellSize, cellSize));
        updateButtonAppearance(button, x, y);

        button.addMouseListener(new MouseAdapter() {
            private CellPosition pressedCell;
            boolean left;
            @Override
            public void mousePressed(MouseEvent e) {
                pressedCell = new CellPosition(x, y);
                if (SwingUtilities.isLeftMouseButton(e))
                    left = true;
            }

            @Override
            public void mouseReleased(MouseEvent e) {
                if (pressedCell != null && button.contains(e.getPoint())) {
                    if (left)
                        controller.handleOpenCell(x, y);
                    else
                        controller.handleToggleFlag(x, y);
                }
                pressedCell = null;
            }
        });

        return button;
    }

    private void updateButtonAppearance(JButton button, int x, int y) {
        Cell cell = model.getCell(x, y);
        button.setIcon(null);
        button.setBackground(null);

        if (cell.isOpened()) {
            if (cell.isMine()) {
                button.setIcon(loadIcon("bomb.png"));
                button.setBackground(Color.RED);
            } else {
                button.setIcon(loadNumberIcon(cell.getMinesAround()));
            }
        } else if (cell.isFlagged()) {
            button.setIcon(loadIcon("flaged.png"));
        } else {
            button.setIcon(loadIcon("closed.png"));
        }
    }

    private ImageIcon loadIcon(String path) {
        ImageIcon icon = new ImageIcon(getClass().getResource("/" + path));
        Image scaledImage = icon.getImage().getScaledInstance(cellSize, cellSize, Image.SCALE_SMOOTH);
        return new ImageIcon(scaledImage);
    }

    private Image getImage(String path) {
        ImageIcon icon = new ImageIcon(getClass().getResource("/" + path));
        return icon.getImage().getScaledInstance(cellSize, cellSize, Image.SCALE_SMOOTH);
    }

    private ImageIcon loadNumberIcon(int number) {
        String[] icons = {
                "zero.png", "num1.png", "num2.png",
                "num3.png", "num4.png", "num5.png",
                "num6.png", "num7.png", "num8.png"
        };
        return loadIcon(icons[number]);
    }

    @Override
    public void update() {
        SwingUtilities.invokeLater(() -> {
            rebuildGameField();
            flagsLabel.setText("Flags: " + model.getFlagsLeft());
        });
    }

    @Override
    public void showError(String message) {
        JOptionPane.showMessageDialog(this, message, "Error", JOptionPane.ERROR_MESSAGE);
    }

    @Override
    public void showGameOver() {
        JOptionPane.showMessageDialog(this, "Game Over!", "Defeat", JOptionPane.INFORMATION_MESSAGE);
        updateTime(0);
    }

    @Override
    public void showVictory() {
        String difficulty = model.getDifficulty();
        int time = model.getTime();

        // Проверка и сохранение рекорда
        if (controller.isNewHighScore(difficulty, time)) {
            controller.updateHighScore(user.getNickname(), difficulty, time);
            JOptionPane.showMessageDialog(
                    this,
                    "New high score for " + user.getNickname() + "! Time: " + time + " secs.",
                    "Congratulations!",
                    JOptionPane.INFORMATION_MESSAGE
            );
        } else {
            JOptionPane.showMessageDialog(
                    this,
                    "Victory! Time: " + time + " secs.",
                    "Congratulations!",
                    JOptionPane.INFORMATION_MESSAGE
            );
        }
        updateTime(0);
    }
}
