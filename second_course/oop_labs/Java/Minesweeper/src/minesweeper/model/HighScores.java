package minesweeper.model;

import java.io.*;
import java.util.*;

public class HighScores {
    private static final String FILE_NAME = "highscores.txt";

    //difficulty → list with records
    private static Map<String, List<ScoreEntry>> scores = new HashMap<>();

    public HighScores() {
        loadScores();
    }

    // Inner class for one record
    static class ScoreEntry {
        String nickname;
        int time;

        ScoreEntry(String nickname, int time) {
            this.nickname = nickname;
            this.time = time;
        }
    }

    private void loadScores() {
        scores.clear();
        try (BufferedReader reader = new BufferedReader(new FileReader(FILE_NAME))) {
            String line;
            while ((line = reader.readLine()) != null) {
                String[] parts = line.split(":");
                if (parts.length == 3) {
                    String difficulty = parts[0];
                    String nickname = parts[1];
                    int time = Integer.parseInt(parts[2]);

                    scores.putIfAbsent(difficulty, new ArrayList<>());
                    scores.get(difficulty).add(new ScoreEntry(nickname, time));
                }
            }
            sortAndTrimScores();
        } catch (IOException e) {
            System.out.println("No previous high scores found.");
        }
    }

    private void saveScores() {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(FILE_NAME))) {
            for (var entry : scores.entrySet()) {
                String difficulty = entry.getKey();
                for (ScoreEntry score : entry.getValue()) {
                    writer.write(difficulty + ":" + score.nickname + ":" + score.time);
                    writer.newLine();
                }
            }
        } catch (IOException e) {
            System.out.println("Error saving high scores.");
        }
    }

    public boolean isNewHighScore(String difficulty, int time, User user) {
        List<ScoreEntry> list = scores.getOrDefault(difficulty, new ArrayList<>());

        Optional<ScoreEntry> userEntry = list.stream()
                .filter(e -> e.nickname.equals(user.getNickname()))
                .findFirst();

        if (userEntry.isPresent()) {
            int currentBest = userEntry.get().time;
            if (time >= currentBest) {
                return false;
            }
        }

        return list.size() < 10 || time < list.getLast().time;
    }

    public void updateHighScore(String nickname, String difficulty, int time, User user) {
        if (!isNewHighScore(difficulty, time, user)) return;

        List<ScoreEntry> entries = scores.getOrDefault(difficulty, new ArrayList<>());

        entries.removeIf(entry -> entry.nickname.equals(nickname));
        entries.add(new ScoreEntry(nickname, time));

        sortAndTrimScores();
        saveScores();
    }

    private void sortAndTrimScores() {
        for (var entry : scores.entrySet()) {
            List<ScoreEntry> list = entry.getValue();
            list.sort(Comparator.comparingInt(e -> e.time));
            if (list.size() > 10) {
                list.subList(10, list.size()).clear();
            }
        }
    }

    public String getHighScores(String difficulty) {
        List<ScoreEntry> list = scores.getOrDefault(difficulty, new ArrayList<>());
        StringBuilder sb = new StringBuilder("Top 10 for " + difficulty + ":\n");
        int rank = 1;
        for (ScoreEntry entry : list) {
            sb.append(rank++).append(". ").append(entry.nickname).append(" - ").append(entry.time).append(" sec\n");
        }
        return sb.toString();
    }
}
