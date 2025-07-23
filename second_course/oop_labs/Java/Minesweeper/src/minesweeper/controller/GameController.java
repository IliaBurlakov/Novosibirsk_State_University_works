package minesweeper.controller;

import minesweeper.model.GameModel;
import minesweeper.model.HighScores;
import minesweeper.model.User;
import minesweeper.view.View;
import minesweeper.view.textui.TextUI;

import javax.swing.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class GameController {
    private User user;
    private HighScores highScores = new HighScores();
    private GameModel model;
    View view;
    private ScheduledExecutorService timerService;
    private AtomicInteger currentTime = new AtomicInteger(0);

    public GameController(GameModel model, View view) {
        this.model = model;
        this.view = view;
    }

    private boolean isValidPosition(int x, int y) {
        return x >= 0 && x < model.getWidth() && y >= 0 && y < model.getHeight();
    }

    public void setUser(User user) {
        this.user = user;
    }

    public void setView(View view) {
        this.view = view;
    }
    public void startTimer() {
        if (timerService != null && !timerService.isShutdown()) return;
        currentTime.set(0);
        timerService = Executors.newSingleThreadScheduledExecutor();
        timerService.scheduleAtFixedRate(() -> {
            int newTime = currentTime.incrementAndGet();
            SwingUtilities.invokeLater(() -> {
                model.setTime(newTime); // Добавьте метод setTime() в GameModel
                view.updateTime(newTime);
            });
        }, 1, 1, TimeUnit.SECONDS);
    }

    public void stopTimer() {
        if (timerService != null) {
            timerService.shutdownNow();
        }
        model.setTime(0);
        currentTime.set(0);
    }
    public int getTime() {
        return currentTime.get();
    }
    public void handleOpenCell(int x, int y) throws IllegalArgumentException {
        if (model.isGameOver() || model.isGameWon()) return;
        try {
            if(!model.isFirstMoveHappened()){
                model.setFirstMoveHappened(true);
                model.generateMines(x, y);
            }
            model.openCell(x, y);
            checkGameStatus();
            view.update();
        }
        catch (IllegalArgumentException | IndexOutOfBoundsException | IllegalStateException e) {
            view.showError(e.getMessage());
        }

    }
    public void handleToggleFlag(int x, int y){
        if (model.isGameOver() || model.isGameWon()) return;
        try {
            model.toggleFlag(x, y);
            checkGameStatus();
            view.update();
        } catch (IllegalStateException | IndexOutOfBoundsException e) {
            view.showError(e.getMessage());
        }
    }

    public boolean isNewHighScore(String difficulty, int time) {
        return highScores.isNewHighScore(difficulty, time, user);
    }

    public void updateHighScore(String nickname, String difficulty, int time) {
        highScores.updateHighScore(nickname, difficulty, time, user);
    }

    public String getHighScores(String difficulty) {
        return highScores.getHighScores(difficulty);
    }

    private void checkGameStatus() {
        if (model.isGameOver()) {
            view.showGameOver();
            stopTimer();
        }
        if (model.isGameWon()) {
            view.showVictory();

            if (user != null) {
                String difficulty = model.getDifficulty();
                int time = getTime();

                if (highScores.isNewHighScore(difficulty, time, user)) {
                    highScores.updateHighScore(user.getNickname(), difficulty, time, user);
                    view.showMessage("New high score for " + user.getNickname() + " in " + difficulty + ": " + time + " sec!");
                }
            }
            stopTimer();
        }
    }


    public void handleNewGame() {
        model.reset();
        stopTimer();
        startTimer();
        view.update();
    }

}
