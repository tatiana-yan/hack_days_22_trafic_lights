import Foundation
import CoreLocation
import CoreGraphics
import MyPlayground_Sources

extension CGPoint {
    func squareDistance(to point: CGPoint) -> CGFloat {
        return self.x.distance(to: point.x) + self.y.distance(to: point.y)
    }
}

// UNUSED
public class Scheduler {
    let tl: TL
    
    init(tl: TL) {
        self.tl = tl
    }
    
    func getPassingPoint(trace: TraceData) -> TracePoint {
        for (idx, item) in trace.enumerated() {
            guard idx + 1 < trace.count else {
                return item
            }
            let stepDistance = item.coordinate.squareDistance(to: trace[idx+1].coordinate)
            let tlDistance = tl.coordinate.squareDistance(to: trace[idx].coordinate)
            if stepDistance > tlDistance {
                return item
            }
        }
        return trace.first! // assert empty trace
    }
    
    func getStopPoints(trace: TraceData) -> [TracePoint] {
        // or just filter all points that have 0 speed
        var stillPoints: [TracePoint] = []
        for (idx, item) in trace.enumerated() {
            guard idx + 1 < trace.count else {
                break
            }
            if item.coordinate == trace[idx+1].coordinate {
                stillPoints.append(item)
            }
        }
        return stillPoints
    }
    
    public func appendSchedule(current schedule: TLSchedule, for turn: Turn, trace: TraceData) -> TLSchedule {
        guard !trace.isEmpty else {
            return schedule
        }
        
        // find "active point" - when trace intersects TL | when trace stops before TL
        let passPoint = self.getPassingPoint(trace: trace)
        let stopPoints = self.getStopPoints(trace: trace)
        
        // find affected frame(s)
//        var passFrame: (Frame, TimeInterval) = (Frame(checkpoint: passPoint.timestamp), passPoint.timestamp)
//        var stopFrames: [(Frame, TimeInterval)] = []
//        for (idx, frame) in schedule.frames.enumerated() {
//            if frame.checkpoint > passPoint.timestamp && idx > 0 {
//                passFrame = (schedule.frames[idx-1], passPoint.timestamp)
//            }
//
//            stopPoints.forEach {
//                if frame.checkpoint > $0.timestamp && idx > 0 {
//                    stopFrames.append((schedule.frames[idx-1], $0.timestamp))
//                }
//            }
//        }
        
        schedule.applyPointIfNeeded(by: passPoint.timestamp, isGreen: true)
        stopPoints.forEach {
            schedule.applyPointIfNeeded(by: $0.timestamp, isGreen: false)
        }
        
        return schedule
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

let traces = Array< (Turn, TraceData) >()
let light = TL(id: TLID(1),
               coordinate: .init(x: 0,
                                 y: 0))

let traceData = generateTraceData()
let linearScheduler = LinearScheduler(data: traceData)
let linearSchedule = linearScheduler.getSchedule(for: 1)

for frame in linearSchedule.frames {
    print(frame)
}
