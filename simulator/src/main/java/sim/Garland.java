package sim;

import java.util.ArrayList;
import java.util.List;

public class Garland {
    private static final int BASE_MODES = 4;
    private static final int MAX_MODES = 8;

    private static class Mode {
        List<Step> steps;
        int curStep = 0;

        Mode(List<Step> steps) {
            this.steps = new ArrayList<>(steps);
        }
    }

    private final Mode[] modes = new Mode[MAX_MODES];
    private int curMode = 0;
    private int modesCount = BASE_MODES;
    private int nextUserSlot = 0;
    private long lastTick = System.currentTimeMillis();

    public Garland() {
        // Режим 1: бегущий огонь
        modes[0] = new Mode(List.of(
                new Step(true, false, false, 250),
                new Step(false, true, false, 250),
                new Step(false, false, true, 250),
                new Step(false, true, false, 250)
        ));

        // Режим 2: стробоскоп
        modes[1] = new Mode(List.of(
                new Step(true, false, true, 70),
                new Step(false, false, false, 70),
                new Step(true, false, true, 70),
                new Step(false, false, false, 180),
                new Step(false, true, false, 70),
                new Step(false, false, false, 70),
                new Step(false, true, false, 70),
                new Step(false, false, false, 250)
        ));

        // Режим 3: по парам
        modes[2] = new Mode(List.of(
                new Step(true, true, false, 500),
                new Step(false, false, false, 200),
                new Step(true, false, true, 500),
                new Step(false, false, false, 200)
        ));

        // Режим 4: накопление
        modes[3] = new Mode(List.of(
                new Step(true, false, false, 350),
                new Step(true, true, false, 350),
                new Step(false, false, false, 250),
                new Step(true, false, false, 350),
                new Step(true, false, true, 350),
                new Step(false, false, false, 250)
        ));

        for (int i = BASE_MODES; i < MAX_MODES; i++) {
            modes[i] = new Mode(List.of());
        }
    }

    public synchronized boolean update() {
        Mode m = modes[curMode];
        if (m.steps.isEmpty()) {
            return false;
        }

        Step s = m.steps.get(m.curStep);
        long now = System.currentTimeMillis();

        if (now - lastTick >= s.delayMs()) {
            m.curStep = (m.curStep + 1) % m.steps.size();
            lastTick = now;
            return true;
        }
        return false;
    }

    public synchronized void nextMode() {
        curMode = (curMode + 1) % modesCount;
        lastTick = System.currentTimeMillis();
    }

    public synchronized boolean setMode(int modeNum) {
        if (modeNum < 1 || modeNum > modesCount) {
            return false;
        }
        curMode = modeNum - 1;
        lastTick = System.currentTimeMillis();
        return true;
    }

    public static boolean checkPattern(String seq) {
        if (seq.length() < 2 || seq.length() > 8) {
            return false;
        }
        for (char c : seq.toCharArray()) {
            if (c != 'g' && c != 'r' && c != 'y' && c != 'n') {
                return false;
            }
        }
        return true;
    }

    public synchronized int addMode(String seq, int delayMs) {
        int slot = nextUserSlot;
        int idx = BASE_MODES + slot;

        List<Step> newSteps = new ArrayList<>();
        for (char c : seq.toCharArray()) {
            newSteps.add(new Step(c == 'g', c == 'y', c == 'r', delayMs));
        }

        modes[idx] = new Mode(newSteps);

        if (modesCount < MAX_MODES) {
            modesCount++;
        }
        nextUserSlot = (nextUserSlot + 1) % 4;

        if (curMode == idx) {
            lastTick = System.currentTimeMillis();
        }

        return idx + 1; // возвращаем номер 5..8
    }

    public synchronized Step getCurrentStep() {
        Mode m = modes[curMode];
        return m.steps.get(m.curStep);
    }

    public synchronized int getCurrentModeNum() {
        return curMode + 1;
    }

    public synchronized int getCurrentStepIndex() {
        return modes[curMode].curStep + 1;
    }

    public synchronized int getCurrentModeStepsCount() {
        return modes[curMode].steps.size();
    }

    public synchronized int getModesCount() {
        return modesCount;
    }
}