import Foundation
import CoreLocation
import CoreGraphics

public typealias TLID = UInt
public typealias RoadID = UInt
public typealias Turn = (from: RoadID, to: RoadID)
public typealias TracePoint = (coordinate: CGPoint, timestamp: TimeInterval)
public typealias TraceData = Array<TracePoint>

public typealias DataPoint = (timestamp: TimeInterval, isGreen: Bool)

func getLight(idx: Int, green: Float, red: Float) -> Bool {
    let isGreen = Float(idx).truncatingRemainder(dividingBy: green + red) < green
    return Int.random(in: 0..<100) > 3 ? isGreen : !isGreen
//    return isGreen
}

public func generateTraceData() -> [DataPoint] {
    let mult = 100
    var trace = [DataPoint]()
    for i in 0..<60*mult {
        var isGreen = true
        switch i {
        case 15*mult..<20*mult:
            fallthrough
        case 40*mult..<45*mult:
            isGreen = getLight(idx: i, green: 100, red: 100)
        case 20*mult..<40*mult:
            fallthrough
        case 45*mult..<60*mult:
            isGreen = getLight(idx: i, green: 150, red: 60)
        default:
            isGreen = true
        }
        trace.append((TimeInterval(i), isGreen))
    }
    return trace
}

public struct Frame {
    public enum FrameOrder {
        case greenFirst
        case redFirst
        case outOfService
    }
    public enum WeekDay {
        case Monday
        case Tuesday
        case Wednesday
        case Thursday
        case Friday
        case Saturday
        case Sunday
    }
    public let day: WeekDay
    public let checkpoint: TimeInterval
    public var greenDuration: TimeInterval
    public var redDuration: TimeInterval
    public let signalOrder: FrameOrder
    
    public init(checkpoint: TimeInterval, signalOrder: FrameOrder = .greenFirst) {
        self.day = .Monday // pick autmoatically
        self.checkpoint = checkpoint
        self.greenDuration = 0 // evaluate good default value
        self.redDuration = 0
        self.signalOrder = signalOrder
    }
    
    /// UNUSED
    public func timeToGreen(from probeTime: TimeInterval) -> TimeInterval {
        guard checkpoint <= probeTime else {
            return TimeInterval.nan
        }
        let relativeInterval = probeTime - checkpoint
        let period: Int = Int(relativeInterval / (greenDuration+redDuration))
        let cycle = relativeInterval - (greenDuration+redDuration) * Double(period)
        switch signalOrder {
        case .greenFirst:
            if cycle < greenDuration {
                return 0
            } else {
                return (redDuration + greenDuration) - cycle
            }
        case .redFirst:
            if cycle < redDuration {
                return 0
            } else {
                return (redDuration + greenDuration) - cycle
            }
        case .outOfService:
            return TimeInterval.nan
        }
    }
}
public class TLSchedule {
    public let tlid: TLID
    public var frames: Array<Frame>
    
    public init(tlid: TLID, frames: Array<Frame>) {
        self.tlid = tlid
        self.frames = frames
    }
    
    /// UNUSED
    public func frame(by date: TimeInterval) -> Frame? {
        for (idx, frame) in frames.enumerated() {
            if frame.checkpoint > date && idx > 0 {
                return frames[idx-1]
            }
        }
        return nil
    }
    
    ///UNUSED
    public func applyPointIfNeeded(by date: TimeInterval, isGreen: Bool) {
        
        if let existingFrame = frame(by: date) {
            let timeToGreen = existingFrame.timeToGreen(from: date)
            if isGreen {
                if timeToGreen == 0 {
                    // fits the schedule; we're happy / harden the weight
                } else {
                    // do something! #heuristics
                }
            } else {
                if timeToGreen != 0 {
                    // we're on red and that's what we wanted \ harden the weight
                } else {
                    // do something! #heuristics
                }
            }
        } else { // no current frame
            frames.insert(Frame(checkpoint: date,
                                signalOrder: isGreen ? .greenFirst : .redFirst),
                          at: 0)
        }
        // check frame fits
        // adjust frame / create new / merge adjacent
    }
}

public class TL {
    public let id: TLID
    public let coordinate: CGPoint

    public init(id: TLID, coordinate: CGPoint) {
        self.id = id
        self.coordinate = coordinate
    }
}


