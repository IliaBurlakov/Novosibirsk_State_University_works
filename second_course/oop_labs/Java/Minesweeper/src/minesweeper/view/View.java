package minesweeper.view;

public interface View {
    void update();
    void showError(String message);
    void showGameOver();
    void showVictory();
    void updateTime(int time);
    void showMessage(String s);
}
