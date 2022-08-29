import Foundation

public class LinearScheduler {
    let data: [DataPoint]
    
    public init(data: [DataPoint]) {
        self.data = data
    }
    
    public func getSchedule(for tlID: TLID) -> TLSchedule {
        return .init(tlid: tlID,
                     frames: process())
    }
    
    enum Mode {
        case green
        case red
    }
    
    func windowValue(at index: Int, _ size: Int) -> Mode {
        var from = index - size/2
        var to = index + size/2
        if size % 2 == 0 {
            from += 1
        }
        from = max(0, from)
        to = min(to, data.count)
        
        let greens = Double(data[from..<to].map({Int(truncating: $0.isGreen as NSNumber)}).reduce(0, +))
        return greens / Double(size) >= 0.5 ? Mode.green : Mode.red
    }
    func process() -> [Frame] {
        let windowSize = 6
        let durationThreshold = 11.0
        
        var previousMode = windowValue(at: 0, windowSize)
        var modeBegin = data.first!.timestamp
        var currentFrame = Frame(checkpoint: data.first!.timestamp,
                                 signalOrder: previousMode == .green ? .greenFirst : .redFirst)
        var frames: [Frame] = []
        for (idx, item) in data.enumerated() {
            let currentMode = windowValue(at: idx, windowSize)
            
            if previousMode != currentMode { // start new frame or update red|green duration
                let delta = item.timestamp - modeBegin
                switch previousMode {
                case .green:
                    if currentFrame.greenDuration == 0 {
                        currentFrame.greenDuration = delta
                    } else if abs(currentFrame.greenDuration - delta) < durationThreshold {
                        currentFrame.greenDuration = (currentFrame.greenDuration + delta) / 2
                    } else {
                        frames.append(currentFrame)
                        currentFrame = Frame(checkpoint: modeBegin,
                                             signalOrder: .greenFirst)
                        currentFrame.greenDuration = delta
                    }
                case .red:
                    if currentFrame.redDuration == 0 {
                        currentFrame.redDuration = delta
                    } else if abs(currentFrame.redDuration - delta) < durationThreshold {
                        currentFrame.redDuration = (currentFrame.redDuration + delta) / 2
                    } else {
                        frames.append(currentFrame)
                        currentFrame = Frame(checkpoint: modeBegin,
                                             signalOrder: .redFirst)
                        currentFrame.redDuration = delta
                    }
                }
                
                previousMode = currentMode
                modeBegin = item.timestamp
            }
        }
        frames.append(currentFrame)
        return frames
    }
}
