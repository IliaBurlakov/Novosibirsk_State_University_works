package minesweeper.view.textui;

import minesweeper.controller.GameController;
import minesweeper.model.GameModel;
import minesweeper.model.User;
import minesweeper.view.GameSettings;
import minesweeper.view.View;
import java.util.Scanner;

public class TextUI implements View {
    private GameModel model;
    private GameController controller;
    private User user;
    private GameSettings settings;
    private Scanner scanner;
    int currentTime;

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        int width = 9;
        int height = 9;
        int mines = 10;
        GameSettings settings = new GameSettings();
        configureSettings(settings);


        GameModel model = new GameModel(settings.getWidth(), settings.getHeight(), settings.getMines(), settings.getDifficulty());
        TextUI textUI = new TextUI(model);
        GameController controller = new GameController(model, textUI);
        textUI.setController(controller);
        textUI.start();
    }

    public TextUI(GameModel model) {
        this.model = model;
        this.scanner = new Scanner(System.in);
    }

    @Override
    public void updateTime(int time) {
        currentTime = time;
    }

    @Override
    public void showMessage(String s) {
        System.out.println(s);
    }

    private static void configureSettings(GameSettings settings) {
        Scanner scanner = new Scanner(System.in);
        System.out.println("Choose difficulty (easy, medium, hard) or enter custom size (e.g., '10 15'):");
        String input = scanner.nextLine().trim().toLowerCase();

        try {
            if (input.matches("\\d+ \\d+")) {
                String[] parts = input.split(" ");
                int width = Integer.parseInt(parts[0]);
                int height = Integer.parseInt(parts[1]);
                settings.setCustom(width, height, 0.2); // 20% мин
            } else {
                settings.setDifficulty(input);
            }
        } catch (Exception e) {
            System.out.println("Invalid input. Using default settings (easy).");
            settings.setDifficulty("easy");
        }
    }

    @Override
    public void update() {
        printField();
    }

    @Override
    public void showError(String message) {
        System.out.println("Error: " + message);
    }

    @Override
    public void showGameOver() {
        System.out.println("Game Over! All mines exploded.");
    }

    @Override
    public void showVictory() {
        if (user != null) {
            String difficulty = model.getDifficulty();
            int time = currentTime;

            if (controller.isNewHighScore(difficulty, time)) {
                controller.updateHighScore(user.getNickname(), difficulty, time);
                System.out.println("New high score for " + user.getNickname() + " in " + difficulty + ": " + time + " sec!");
            }
        }

    }

    public void start() {
        System.out.println("Welcome to Minesweeper!");
        System.out.println("Enter your nickname:");
        String nickname = scanner.nextLine().trim();
        if (nickname == null || nickname.isEmpty()) {
            nickname = "Anonymous";
        }
        user = new User(nickname);
        this.controller.setUser(user);
        System.out.println("Welcome, " + user.getNickname() + "!");
        controller.startTimer();
        printField();
        handleInput();
    }

    private void setController(GameController controller) {
        this.controller = controller;
    }

    private void printField() {
        System.out.println("\nTime: " + model.getTime() + " secs.");
        System.out.println("\nFlags left: " + model.getFlagsLeft());
        System.out.print("   ");
        for (int x = 0; x < model.getWidth(); x++) {
            System.out.print(x + " ");
        }
        System.out.println();

        for (int y = 0; y < model.getHeight(); y++) {
            System.out.print(y + " |");
            for (int x = 0; x < model.getWidth(); x++) {
                var cell = model.getCell(x, y);
                if (cell.isOpened()) {
                    System.out.print(cell.isMine() ? "* " : cell.getMinesAround() + " ");
                } else if (cell.isFlagged()) {
                    System.out.print("F ");
                } else {
                    System.out.print(". ");
                }
            }
            System.out.println("|");
        }
    }

    private void handleInput() {
        while (true) {
            if (model.isGameOver() || model.isGameWon()) {
                System.out.println("Game is over. Enter command: new, about, exit");
            } else {
                System.out.println("\nEnter command: open x y, flag x y, highscores, new, about, exit:");
            }

            String input = scanner.nextLine().trim().toLowerCase();
            String[] parts = input.split(" ");

            try {
                switch (parts[0]) {
                    case "open":
                        if (parts.length == 3) {
                            int x = Integer.parseInt(parts[1]);
                            int y = Integer.parseInt(parts[2]);
                            controller.handleOpenCell(x, y);
                        }
                        break;
                    case "flag":
                        if (parts.length == 3) {
                            int x = Integer.parseInt(parts[1]);
                            int y = Integer.parseInt(parts[2]);
                            controller.handleToggleFlag(x, y);
                        }
                        break;
                    case "highscores":
                        System.out.println("Select difficulty (Easy, Medium, Hard):");
                        String difficulty = scanner.nextLine().trim().toLowerCase();
                        System.out.println(controller.getHighScores(difficulty));
                        break;
                    case "new":
                        controller.handleNewGame();
                        break;
                    case "about":
                        showAboutInfo();
                        break;
                    case "exit":
                        System.exit(0);
                    default:
                        System.out.println("Unknown command!");
                }
            } catch (NumberFormatException e) {
                showError("Invalid number format!");
            } catch (Exception e) {
                showError(e.getMessage());
            }
        }
    }
    private void showAboutInfo() {
        String aboutText = """
        Minesweeper v1.0
        Created by Ilia Burlakov
        2025
        """;
        System.out.println(aboutText);
    }
}