'use client'

interface MarketData {
  market_id: string
  market: number
  event_name: string
  best_bid: number
  best_ask: number
  bid_size: number
  ask_size: number
}

interface MarketDataViewProps {
  marketData: MarketData[]
}

const marketNames: { [key: number]: string } = {
  0: 'Polymarket',
  1: 'Kalshi',
  2: 'PredictIt'
}

export default function MarketDataView({ marketData }: MarketDataViewProps) {
  return (
    <div className="card">
      <h2>Market Data</h2>
      {marketData.length === 0 ? (
        <p style={{ color: '#999' }}>No market data received yet...</p>
      ) : (
        <div className="market-data">
          {marketData.map((data) => (
            <div key={data.market_id} className="market-item">
              <h3>{data.event_name}</h3>
              <div style={{ fontSize: '0.875rem', color: '#999', marginBottom: '0.5rem' }}>
                {marketNames[data.market] || `Market ${data.market}`} - {data.market_id}
              </div>
              <div className="price">
                ${((data.best_bid + data.best_ask) / 2).toFixed(4)}
              </div>
              <div className="bid-ask">
                <span>Bid: ${data.best_bid.toFixed(4)}</span>
                <span>Ask: ${data.best_ask.toFixed(4)}</span>
              </div>
              <div className="bid-ask" style={{ marginTop: '0.25rem' }}>
                <span>Size: ${data.bid_size.toFixed(2)}</span>
                <span>Size: ${data.ask_size.toFixed(2)}</span>
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  )
}

